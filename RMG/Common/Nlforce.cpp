/*
 *
 * Copyright 2014 The RMG Project Developers. See the COPYRIGHT file 
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

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "const.h"
#include "RmgTimer.h"
#include "rmgtypedefs.h"
#include "params.h"
#include "typedefs.h"
#include "common_prototypes.h"
#include "common_prototypes1.h"
#include "rmg_error.h"
#include "Kpoint.h"
#include "prototypes.h"


#include "TradeImages.h"
#include "RmgException.h"
#include "Lattice.h"
#include "FiniteDiff.h"
#include "transition.h"



/*Set this to 1 to write out true NL force and the part
 * that comes from eigenvalues*/
#define VERBOSE 0

template void Nlforce<double> (double *, Kpoint<double> **Kptr);
template void Nlforce<std::complex<double> > (double * , Kpoint<std::complex<double>> **Kptr);

template <typename OrbitalType> void Nlforce (double * veff, Kpoint<OrbitalType> **Kptr)
{
    int ion, isp, index, gion, nion;
    int nh, size, size1;
    double *gamma, *par_gamma, *par_omega;
    SPECIES *sp;
    ION *iptr;
    int num_ions;
    fftw_plan p2;
    std::complex<double> *in, *out;
    double *newsintR_x, *newsintR_y, *newsintR_z, *qforce;
    double *newsintI_x, *newsintI_y, *newsintI_z, *tmp_force_gamma, *tmp_force_omega;
    int fpt0;
    RmgTimer *RT1;
#if VERBOSE
    double *old_force, sum1x, sum1y, sum1z, sum2x, sum2y, sum2z;
    old_force = new double[ 3 * ct.num_ions];
#endif


    size1 = ct.num_kpts * ct.num_states * ct.num_ions * ct.max_nl;
    fpt0 = ct.fpt[0];


    num_ions = ct.num_ions;


    /*Array for q-force */
    qforce = new double[ 3 * num_ions];
    tmp_force_gamma = new double[ 3 * num_ions];
    tmp_force_omega = new double[ 3 * num_ions];

    for (isp = 0; isp < 3 * num_ions; isp++)
    {
        qforce[isp] = 0.0;
        tmp_force_gamma[isp] = 0.0;
        tmp_force_omega[isp] = 0.0;
    }


    /*max for nh * (nh + 1) / 2 */
    size = (ct.max_nl + 1) * ct.max_nl / 2;
    
    gamma = new double[ size];
    par_gamma = new double[ 6 * size];
    par_omega = par_gamma + 3 * size;


    RT1 = new RmgTimer("Force: non-local: betaxpsi");
    for (int kpt = 0; kpt < ct.num_kpts; kpt++)
    {
        Betaxpsi(Kptr[kpt], 0, Kptr[kpt]->nstates, Kptr[kpt]->sint_derx, Kptr[kpt]->nl_weight_derx);
        Betaxpsi(Kptr[kpt], 0, Kptr[kpt]->nstates, Kptr[kpt]->sint_dery, Kptr[kpt]->nl_weight_dery);
        Betaxpsi(Kptr[kpt], 0, Kptr[kpt]->nstates, Kptr[kpt]->sint_derz, Kptr[kpt]->nl_weight_derz);
    }
    delete RT1;

    /*Loop over ions to setup newsintR_*, this loop is done separately to 
     * insure proper paralelization*/
    double *gx, *gy, *gz;
    double hxxgrid, hyygrid, hzzgrid;

    int FPX0_GRID, FPY0_GRID, FPZ0_GRID, FP0_BASIS;

    hxxgrid = get_hxxgrid();
    hyygrid = get_hyygrid();
    hzzgrid = get_hzzgrid();

    FPX0_GRID = get_FPX0_GRID();
    FPY0_GRID = get_FPY0_GRID();
    FPZ0_GRID = get_FPZ0_GRID();
    FP0_BASIS = FPX0_GRID * FPY0_GRID * FPZ0_GRID;



    gx = new double[FP0_BASIS];
    gy = new double[FP0_BASIS];
    gz = new double[FP0_BASIS];

    CPP_app_grad_driver (&Rmg_L, Rmg_T, veff, gx, gy, gz, FPX0_GRID, FPY0_GRID, FPZ0_GRID, hxxgrid, hyygrid, hzzgrid, 6);


    for (ion = 0; ion < pct.num_nonloc_ions; ion++)
    {
        /*Actual index of the ion under consideration*/
        gion = pct.nonloc_ions_list[ion];
        
        iptr = &ct.ions[gion];

        nh = ct.sp[iptr->species].nh;

        RT1 = new RmgTimer("Force: non-local get gamma");
        GetGamma (gamma, ion, nh, Kptr);
        delete RT1;
        RT1 = new RmgTimer("Force: non-local nlforce_par_Q");
        nlforce_par_Q (gx, gy, gz, gamma, gion, iptr, nh, &qforce[3 * gion]);
        delete RT1;

    }                           /*end for(ion=0; ion<ions_max; ion++) */

    delete [] gx;
    delete [] gy;
    delete [] gz;



    RT1 = new RmgTimer("Force: non-local global sums");
    size1 = 3 * num_ions;
    global_sums (qforce, &size1, pct.img_comm);
    delete RT1;
        
    
    /*Add force calculated in nlforce1_par_Q */
    for (ion = 0; ion < ct.num_ions; ion++)
    {
        iptr = &ct.ions[ion];

        index = 3 * (ion);
#if VERBOSE  
    printf("\n  Qforce");
    printf ("\n Ion %d Force  %10.7f  %10.7f  %10.7f",
                    ion,
        get_vel_f() * qforce[index],
        get_vel_f() * qforce[index + 1],get_vel_f() * qforce[index + 2]);
#endif
                    
                    

        iptr->force[fpt0][0] += get_vel_f() * qforce[index];
        iptr->force[fpt0][1] += get_vel_f() * qforce[index + 1];
        iptr->force[fpt0][2] += get_vel_f() * qforce[index + 2];
    }


    /*Loop over ions again */
    nion = -1;
    for (ion = 0; ion < pct.num_owned_ions; ion++)
    {
        /*Global index of owned ion*/
        gion = pct.owned_ions_list[ion];

        /* Figure out index of owned ion in nonloc_ions_list array, store it in nion*/
        do {

            nion++;
            if (nion >= pct.num_nonloc_ions)
            {
                printf("\n Could not find matching entry in pct.nonloc_ions_list for owned ion %d", gion);
                rmg_error_handler(__FILE__, __LINE__, "Could not find matching entry in pct.nonloc_ions_list for owned ion ");
            }

        } while (pct.nonloc_ions_list[nion] != gion);

        iptr = &ct.ions[gion];


        nh = ct.sp[iptr->species].nh;

        /*partial_gamma(ion,par_gamma,par_omega, iptr, nh, p1, p2); */
        RT1 = new RmgTimer("Force: non-local partial gamma");
        PartialGamma (gion, par_gamma, par_omega, nion, nh, Kptr);
        delete RT1;
        RT1 = new RmgTimer("Force: non-local nlforce_par_gamma");
        nlforce_par_gamma (par_gamma, gion, nh, &tmp_force_gamma[3*gion]);
        delete RT1;

        RT1 = new RmgTimer("Force: non-local nlforce_par_omega");
        nlforce_par_omega (par_omega, gion, nh, &tmp_force_omega[3*gion]);
        delete RT1;

    }                           /*end for(ion=0; ion<num_ions; ion++) */

    size1 = 3 * num_ions;
    RT1 = new RmgTimer("Force: non-local global sums");
    global_sums (tmp_force_gamma, &size1, pct.img_comm);
    global_sums (tmp_force_omega, &size1, pct.img_comm);
    delete RT1;

    for (ion = 0; ion < ct.num_ions; ion++)
    {
        iptr = &ct.ions[ion];

        index = 3 * (ion);
        iptr->force[fpt0][0] += tmp_force_gamma[index];
        iptr->force[fpt0][1] += tmp_force_gamma[index + 1];
        iptr->force[fpt0][2] += tmp_force_gamma[index + 2];
    }



#if VERBOSE
    if (pct.imgpe == 0)
    {
        printf ("\n\n True Non-local forces:");

        for (ion = 0; ion < ct.num_ions; ion++)
        {

            iptr = &ct.ions[ion];
            printf ("\n Ion %d Force  %10.7f  %10.7f  %10.7f",
                    ion,
                    tmp_force_gamma[3 * ion],
                    tmp_force_gamma[3 * ion+1],
                    tmp_force_gamma[3 * ion+2]);

            sum1x += iptr->force[fpt0][0] - old_force[3 * ion];
            sum1y += iptr->force[fpt0][1] - old_force[3 * ion + 1];
            sum1z += iptr->force[fpt0][2] - old_force[3 * ion + 2];
        }
        printf ("\n True NL sum in x, y and z directions: %e %e %e", sum1x, sum1y, sum1z);
    }

#endif


    for (ion = 0; ion < ct.num_ions; ion++)
    {
        iptr = &ct.ions[ion];

        index = 3 * (ion);

#if VERBOSE
        old_force[index] = iptr->force[fpt0][0];
        old_force[index + 1] = iptr->force[fpt0][1];
        old_force[index + 2] = iptr->force[fpt0][2];
#endif

        iptr->force[fpt0][0] += tmp_force_omega[index];
        iptr->force[fpt0][1] += tmp_force_omega[index + 1];
        iptr->force[fpt0][2] += tmp_force_omega[index + 2];
    }


#if VERBOSE
    if (pct.imgpe == 0)
    {
        printf ("\n\n Eigenvalue force:");

        for (ion = 0; ion < ct.num_ions; ion++)
        {

            iptr = &ct.ions[ion];
            printf ("\n Ion %d Force  %10.7f  %10.7f  %10.7f",
                    ion,
                    iptr->force[fpt0][0] - old_force[3 * ion],
                    iptr->force[fpt0][1] - old_force[3 * ion + 1],
                    iptr->force[fpt0][2] - old_force[3 * ion + 2]);

            sum2x += iptr->force[fpt0][0] - old_force[3 * ion];
            sum2y += iptr->force[fpt0][1] - old_force[3 * ion + 1];
            sum2z += iptr->force[fpt0][2] - old_force[3 * ion + 2];
        }
        printf ("\n Eigenvalue force sum in x, y and z directions: %e %e %e", sum2x, sum2y, sum2z);
    }
#endif

    delete[] par_gamma;
    delete[] gamma;
    delete[] tmp_force_gamma;
    delete[] tmp_force_omega;
    delete[] qforce;
    delete[] newsintR_x;
    delete[] newsintI_x;

#if VERBOSE
    delete[] old_force;
#endif

}

