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


#ifndef RMG_Symmetry_H
#define RMG_Symmetry_H 1

#include <vector>
#include <cstdint>
#include "rmgtypedefs.h"
#include "BaseGrid.h"
#include "Lattice.h"
#include <boost/multi_array.hpp>

extern "C" int spg_get_multiplicity(const double *lattice,
        const double *position,
        const int *types,
        const int num_atom,
        const double symprec,
        const double angprec);


extern "C" int spg_get_symmetry(int *rotation,
        double *translation,
        const int max_size,
        const double *lattice,
        const double *position,
        const int types[],
        const int num_atom,
        const double symprec,
        const double angprec);

void symm_ijk(int *srotate, int *strans, int &ix, int &iy, int &iz, 
        int &ixx, int &iyy, int &izz, int &nx, int &ny, int &nz);

class Symmetry
{

    private:
        int max_pdim;

        BaseGrid &G;
        Lattice &L;

        // Grid sizes per MPI task
        int px_grid;
        int py_grid;
        int pz_grid;

        // Global grid sizes
        int nx_grid;
        int ny_grid;
        int nz_grid;

        int xoff;
        int yoff;
        int zoff;

        // basis sizes
        int pbasis;
        int nbasis;

        std::vector<int> sym_atom;

        double symprec;
        double angprec;

        std::vector<uint8_t> sym_index_x8;
        std::vector<uint8_t> sym_index_y8;
        std::vector<uint8_t> sym_index_z8;
        std::vector<uint16_t> sym_index_x16;
        std::vector<uint16_t> sym_index_y16;
        std::vector<uint16_t> sym_index_z16;

        template <typename T>
            void init_symm_ijk(std::vector<T> &sym_x_idx, std::vector<T> &sym_y_idx, std::vector<T> &sym_z_idx);

        template <typename T, typename U>
            void symmetrize_grid_object_int(T *object, const std::vector<U> &sym_x_idx, const std::vector<U> &sym_y_idx, const std::vector<U> &sym_z_idx);
        template <typename U>
            void symmetrize_grid_vector_int(double *object, const std::vector<U> &sym_x_idx, const std::vector<U> &sym_y_idx, const std::vector<U> &sym_z_idx);

    public:
        int nsym;
        std::vector<bool> inv_type;
        std::vector<bool> time_rev;
        std::vector<int> ftau;
        std::vector<int> ftau_wave;
        boost::multi_array<double, 4> rot_ylm;
        boost::multi_array<std::complex<double>, 3> rot_spin;
        Symmetry(BaseGrid &G_in, Lattice &L_in, int density);
        ~Symmetry(void);

        std::vector<int> sym_rotate;
        std::vector<double> sym_trans;

        template <typename T>
            void symmetrize_grid_object(T *object);

        void symmetrize_grid_vector(double *mag_rho);
        void symm_vec(int isy, double *vec);
        void symforce(void);
        void symmetrize_tensor(double *mat_tensor);
        void rotate_ylm();
        void rotate_spin();
        int type_symm(double sr[3][3]);
        void symm_nsocc(std::complex<double> *ns_occ, int mmax);


};
#endif
