/*
 *
 * Copyright 2019 The RMG Project Developers. See the COPYRIGHT file 
 * at the top-level directory of this distribution or in the current
 * directory.
 * 
 * This file is part of RMG. 
 * RMG is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * any later version.
 *
 * RMG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
*/

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iterator>
#include <omp.h>


#include "const.h"
#include "Exxbase.h"
#include "RmgTimer.h"
#include "RmgException.h"
#include "RmgGemm.h"
#include "transition.h"
#include "rmgtypedefs.h"
#include "pe_control.h"
#include "GpuAlloc.h"
#include "Exx_on.h"
#include "prototypes_on.h"

// This class implements exact exchange for ON module using the Exxbase clase
// DelocalOrbital: is delocalized and store K_kl * Phi_l(r), 
// K_kl is the charge density matrix, C_ki * f(i) C_il =  mat_X from DaigScalapack.cpp
// K_kl dimension: num_tot * num_thispe for LocalOrbital


template Exx_on<double>::Exx_on(BaseGrid &, BaseGrid &, Lattice &, const std::string &, LocalObject<double> &phi, int);
template Exx_on<std::complex<double>>::Exx_on(BaseGrid &, BaseGrid &, Lattice &, const std::string &, 
    LocalObject<std::complex<double>> &phi, int );

template Exx_on<double>::~Exx_on(void);
template Exx_on<std::complex<double>>::~Exx_on(void);
template <class T> Exx_on<T>::~Exx_on()
{
};

template <class T> Exx_on<T>::Exx_on (
          BaseGrid &G_in,
          BaseGrid &G_h_in,
          Lattice &L_in,
          const std::string &wavefile_in,
          LocalObject<T> &Phi_in, 
          int mode_in) : G(G_in), G_h(G_h_in), L(L_in), wavefile(wavefile_in), Phi(Phi_in), mode(mode_in)
{
    RmgTimer RT0("5-Functional: Exx init");
    Exxb = new Exxbase<T>(G, G_h, L, wavefile, Phi.num_tot, NULL, NULL, ct.exx_mode);
    DePhi = (T *)GpuMallocManaged(ct.num_states * ct.num_states * sizeof(T));
    Omega_j = (T *)GpuMallocManaged(Phi.num_thispe * Phi.pbasis * sizeof(T));
    Xij_mat =  (T *)GpuMallocManaged(Phi.num_tot * Phi.num_tot * sizeof(T));
}

template <> void Exx_on<double>::Omega(double *rho_matrix, bool use_float_fft)
{
    double one = 1.0, zero = 0.0;
    int num_tot = Phi.num_tot;
    int num_thispe = Phi.num_thispe;
    int pbasis = Phi.pbasis;
    RmgTimer RT0("5-Functional: Exx potential");

    RmgGemm("N", "N", pbasis, num_tot, num_thispe, one, Phi.storage_proj, pbasis,
            rho_matrix, num_thispe, zero, DePhi, pbasis);


    double scale = - 1.0 / (double)Exxb->pwave->global_basis;

    // Clear vexx
    for(int idx=0;idx < Phi.num_thispe*pbasis;idx++) Omega_j[idx] = 0.0;


    double *zeropsi = new double[pbasis]();
    std::vector<std::complex<double> *> pvec;
    std::vector<std::complex<float> *> wvec;
    pvec.resize(ct.OMP_THREADS_PER_NODE);
    wvec.resize(ct.OMP_THREADS_PER_NODE);
    for(int tid=0;tid < ct.OMP_THREADS_PER_NODE;tid++)
    {
        pvec[tid] = (std::complex<double> *)fftw_malloc(sizeof(std::complex<double>) * Exxb->pwave->pbasis);
        wvec[tid] = (std::complex<float> *)fftw_malloc(sizeof(std::complex<float>) * Exxb->pwave->pbasis);
    }

    if(mode != EXX_DIST_FFT)
    {
        throw RmgFatalException() << "only dist_fft mode for Hybrid now" << "\n";
    }
    // Loop over v_kj, here k, and j are all orbitals
    // v_kj = sum_r DePhi_k(r) * Phi_j(r)
    for(int j=0;j < num_tot;j++)
    {   
        int j_local = Phi.index_global_to_proj[j];
        double *phi_j, *Ome_j;
        if(j_local < 0)
        {
            phi_j = zeropsi;
            Ome_j = zeropsi;
        }
        else
        {
            phi_j = &Phi.storage_proj[j_local * pbasis];
            Ome_j = &Omega_j[j_local * pbasis];
        }
#pragma omp parallel for schedule(dynamic)
        for(int k=0; k < num_tot;k++)
        {
            double *psi_k = (double *)&DePhi[k*pbasis];
            int k_local = Phi.index_global_to_proj[k];
            double *phi_k;
            if(k_local < 0)
                phi_k = zeropsi;
            else
                phi_k = &Phi.storage_proj[k_local * pbasis];

            int omp_tid = omp_get_thread_num();
            std::complex<double> *p = pvec[omp_tid];
            std::complex<float> *w = wvec[omp_tid];

            RmgTimer RT1("5-Functional: Exx potential fft");

            if(use_float_fft)
            {
                for(int idx = 0; idx < pbasis; idx++) w[idx] = std::complex<float>(psi_k[idx] * phi_j[idx], 0.0);
                Exxb->pwave->FftForward(w, w);
                for(int ig=0;ig < pbasis;ig++) w[ig] *= Exxb->gfac[ig];
                Exxb->pwave->FftInverse(w, w);
                for(int ig=0;ig < pbasis;ig++) p[ig] = w[ig];
            }
            else
            {
                for(int idx = 0; idx < pbasis; idx++) p[idx] = std::complex<double>(psi_k[idx] * phi_j[idx], 0.0);
                Exxb->pwave->FftForward(p, p);
                for(int ig=0;ig < pbasis;ig++) p[ig] *= Exxb->gfac[ig];
                Exxb->pwave->FftInverse(p, p);
            }

#pragma omp critical(part1)
            {               
                if(j_local >= 0 && k_local >= 0)
                    for(int idx = 0;idx < pbasis;idx++)Ome_j[idx] += scale * std::real(p[idx]) * phi_k[idx];
            }


        }
    }

    for(int tid=0;tid < ct.OMP_THREADS_PER_NODE;tid++)
    {
        fftw_free(wvec[tid]);
        fftw_free(pvec[tid]);
    }
    delete [] zeropsi;
}

template <> void Exx_on<std::complex<double>>::Omega(std::complex<double> *rho_matrix, bool use_fft_float)
{
}

template <> void Exx_on<std::complex<double>>::Xij(LocalObject<std::complex<double>> &Phi){};
template <> void Exx_on<double>::Xij(LocalObject<double> &Phi)
{
    
    double t1 = Rmg_G->get_NX_GRID(Phi.density);
    t1 *= Rmg_G->get_NY_GRID(Phi.density);
    t1 *= Rmg_G->get_NZ_GRID(Phi.density);

    double vol = Rmg_L.get_omega() /t1;

    int na = Phi.num_thispe ;

    int pbasis = Phi.pbasis;

    double zero = 0.0;

    double *mat_local = (double *)GpuMallocManaged(na*na*sizeof(double));
    RmgGemm("C", "N", na, na, pbasis, vol, Phi.storage_proj, pbasis,
               (double *)Omega_j, pbasis, zero, mat_local, na);

    mat_local_to_glob(mat_local, Xij_mat, Phi, Phi, 0, Phi.num_tot, 0, Phi.num_tot, true);

    GpuFreeManaged(mat_local);

};

template <> double Exx_on<std::complex<double>>::Exxenergy(std::complex<double> * )
{return 0.0;};
template <> double Exx_on<double>::Exxenergy(double *rho_mat_global)
{
    double exx = 0.0;
    for(int idx = 0; idx < Phi.num_tot * Phi.num_tot; idx++) exx += rho_mat_global[idx] * this->Xij_mat[idx];
    return exx;
}
