/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/
 
/****f* QMD-MGDFT/get_vh_soft.c *****
 * NAME
 *   Ab initio real space code with multigrid acceleration
 *   Quantum molecular dynamics package.
 *   Version: 2.1.5
 * COPYRIGHT
 *   Copyright (C) 1995  Emil Briggs
 *   Copyright (C) 1998  Emil Briggs, Charles Brabec, Mark Wensell, 
 *                       Dan Sullivan, Chris Rapcewicz, Jerzy Bernholc
 *   Copyright (C) 2001  Emil Briggs, Wenchang Lu,
 *                       Marco Buongiorno Nardelli,Charles Brabec, 
 *                       Mark Wensell,Dan Sullivan, Chris Rapcewicz,
 *                       Jerzy Bernholc
 * FUNCTION
 *   void get_vh(REAL *rho, REAL *rhoc, REAL *vh_eig, int sweeps, int maxlevel)
 *   Iterative procedure to obtain the hartree potential.
 *   Uses Mehrstallen finite differencing with multigrid accelerations.
 *   The multigrid scheme uses a standard W-cycle with Jacobi relaxation.
 *   (Underrelaxed on the coarser grids.)
 * INPUTS
 *   rho: total charge density
 *   rhoc: compensating charge density
 *   sweeps:  number of iterations 
 *   maxlevel:  maximum multigrid level
 * OUTPUT
 *   vh_eig: Hartree potential
 * PARENTS
 *   init.c scf.c
 * CHILDREN
 *   getpoi_bc.c pack_vhstod.c pack_ptos.c app_cir.c 
 *   set_bc.c app_cil.c mgrid_solv.c pack_vhdtos.c
 * SOURCE
 */



#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "md.h"


/* Pre and post smoothings on each level */
static int poi_pre[5] = { 0, 2, 2, 2, 2 };
static int poi_post[5] = { 0, 2, 2, 2, 2 };


void get_vh_negf (REAL * rho, REAL * rhoc, REAL * vh_eig, REAL * vh_old, int sweeps, int maxlevel)
{

    int idx, its, nits, sbasis, pbasis;
    REAL t1, vavgcor, diag;
    REAL *mgrhsarr, *mglhsarr, *mgresarr, *work;
    REAL *sg_rho, *nrho;
    int stop = FP0_BASIS, ione = 1;
    int i, j, k;
    double tem;

    int incx = 1, cycles;

    double k_vh;

    REAL time1, time2;
    time1 = my_crtc ();

    /* Keep in memory vh*rho_new before updating vh */

    k_vh = 0.0;

    nits = ct.poi_parm.gl_pre + ct.poi_parm.gl_pst;
    sbasis = (ct.vh_pxgrid + 2) * (ct.vh_pygrid + 2) * (ct.vh_pzgrid + 2);
    pbasis = ct.vh_pxgrid * ct.vh_pygrid * ct.vh_pzgrid;

    poi_pre[maxlevel] = 50;     /*shuchun wang */

    /* Grab some memory for our multigrid structures */
    my_malloc_init( mgrhsarr, 9 * sbasis, REAL );
    mglhsarr = mgrhsarr + sbasis;
    mgresarr = mglhsarr + sbasis;
    work = mgresarr + sbasis;
    sg_rho = work + 4 * sbasis;
    nrho = sg_rho + sbasis;



    /* Subtract off compensating charges from rho */
    for (idx = 0; idx < pbasis; idx++)
        work[idx] = rho[idx] - rhoc[idx];


    for (idx = 0; idx < sbasis; idx++)
        nrho[idx] = 0.0;
    pack_vhstod (work, nrho, FPX0_GRID, FPY0_GRID, FPZ0_GRID);

    /* Transfer rho into smoothing grid */
    pack_ptos (sg_rho, nrho, ct.vh_pxgrid, ct.vh_pygrid, ct.vh_pzgrid);

    /* Apply CI right hand side to rho and store result in work array */
    app_cir (sg_rho, mgrhsarr, ct.vh_pxgrid, ct.vh_pygrid, ct.vh_pzgrid);


    /* Multiply through by 4PI */
    t1 = -4.0 * PI;
    QMD_sscal (pbasis, t1, mgrhsarr, incx);


    for (its = 0; its < sweeps; its++)
    {


        /* Mehrstallen smoothings */
        for (cycles = 0; cycles < nits; cycles++)
        {


            /* Transfer vh into smoothing grid */
            pack_ptos (sg_rho, ct.vh_ext, ct.vh_pxgrid, ct.vh_pygrid, ct.vh_pzgrid);

            /* Apply operator */
            diag = app_cil (sg_rho, mglhsarr, ct.vh_pxgrid, ct.vh_pygrid, ct.vh_pzgrid,
                            ct.hxxgrid, ct.hyygrid, ct.hzzgrid);
            diag = -1.0 / diag;

            /* Generate residual vector */
            for (idx = 0; idx < pbasis; idx++)
            {

                mgresarr[idx] = mgrhsarr[idx] - mglhsarr[idx];

            }                   /* end for */

            /*  Fix Hartree in some region  */
            confine (mgresarr, ct.vh_pxgrid, ct.vh_pygrid, ct.vh_pzgrid, potentialCompass, 0);


            /* Pre and Post smoothings and multigrid */
            if (cycles == ct.poi_parm.gl_pre)
            {


                /* Transfer res into smoothing grid */
                pack_ptos (sg_rho, mgresarr, ct.vh_pxgrid, ct.vh_pygrid, ct.vh_pzgrid);


                mgrid_solv_negf (mglhsarr, sg_rho, work,
                            ct.vh_pxgrid, ct.vh_pygrid, ct.vh_pzgrid, ct.hxxgrid,
                            ct.hyygrid, ct.hzzgrid,
                            0, pct.neighbors, ct.poi_parm.levels, poi_pre,
                            poi_post, ct.poi_parm.mucycles, ct.poi_parm.sb_step, k_vh,
                            FG_NX*NX_GRID, FG_NY*NY_GRID, FG_NZ*NZ_GRID,
                            pct.FPX_OFFSET, pct.FPY_OFFSET, pct.FPZ_OFFSET,
                            pct.FPX0_GRID, pct.FPY0_GRID, pct.FPZ0_GRID);


                /* Transfer solution back to mgresarr array */
                pack_stop (mglhsarr, mgresarr, ct.vh_pxgrid, ct.vh_pygrid, ct.vh_pzgrid);

                /*  Fix Hartree in some region  */

                confine (mgresarr, ct.vh_pxgrid, ct.vh_pygrid, ct.vh_pzgrid, potentialCompass, 0);

                /* Update vh */
                t1 = 1.0;
                QMD_saxpy (pbasis, t1, mgresarr, incx, ct.vh_ext, incx);

            }
            else
            {

                /* Update vh */
                t1 = -ct.poi_parm.gl_step * diag;
/*                t1  = 1.0/ct.Ac;*/


                QMD_saxpy (pbasis, t1, mgresarr, incx, ct.vh_ext, incx);

            }                   /* end if */


        }                       /* end for */

    }                           /* end for */


    /* Pack the portion of the hartree potential used by the wavefunctions
     * back into the wavefunction hartree array. */
    pack_vhdtos (vh_eig, ct.vh_ext, FPX0_GRID, FPY0_GRID, FPZ0_GRID);


    /* Release our memory */
    my_free(mgrhsarr);

    time2 = my_crtc ();
    rmg_timings (HARTREE_TIME, (time2 - time1));

}                               /* end get_vh */



/******/
