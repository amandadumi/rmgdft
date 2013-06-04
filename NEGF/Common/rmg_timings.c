/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/
 
/****f* QMD-MGDFT/rmg_timings.c *****
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
 *   void write_timings(void)
 *   some other subroutines to define clock for different platform
 * INPUTS
 *   nothing
 * OUTPUT
 *   print the timeings in the standard output.
 * PARENTS
 *   too many
 * CHILDREN
 *   no
 * SOURCE
 */


#include "main.h"
#include <time.h>
#include <stdio.h>


REAL timings[LAST_TIME];

#if !HYBRID_MODEL
void rmg_timings(int what, REAL time)
{

    timings[what] += time;

}                               /* end rmg_timings */
#endif

#include <sys/time.h>

REAL my_crtc (void)
{
    REAL val, val1;
    struct timeval t1;
    gettimeofday (&t1, NULL);
    val1 = (REAL) t1.tv_usec;
    val1 /= 1000000.0;
    val = (REAL) t1.tv_sec;
    val += val1;
    return val;
}




/* Outputs timing information */
void write_timings (void)
{

    int nn;

    nn = ct.num_steps;

/* Write timing information */
    if (pct.gridpe == 0)
    {


        printf ("\n\n  TIMING INFORMATION\n");
        printf ("*********************************\n");
        printf ("TOTAL                %12.4f\n", timings[TOTAL_TIME]);
        printf ("---------------------------------\n");
        printf ("  INIT               %12.4f\n", timings[INIT_SOFT_TIME]);
        printf ("  QUENCH             %12.4f\n", timings[QUENCH_TIME]);
        printf ("  memory alloc.      %12.4f\n", timings[ALLOC_TIME]);
        printf ("---------------------------------\n");
        printf ("    sigma_all        %12.4f\n", timings[SIGMA_ALL_TIME]);
        printf ("    SCF              %12.4f   (%12.4f /step)\n", timings[SCF_TIME],
                timings[SCF_TIME] / ct.max_scf_steps);
        printf ("---------------------------------\n");
        printf ("      get_ddd        %12.4f\n", timings[GET_DDD_TIME]);
        printf ("      get_Hij        %12.4f\n", timings[GET_Hij_TIME]);
        printf ("      get_matB       %12.4f\n", timings[GET_MATB_SOFT_TIME]);
        printf ("      charge mat     %12.4f\n", timings[CHARGE_DEN_MAT_TIME]);
        printf ("      new rho        %12.4f\n", timings[GET_NEW_RHO]);
        printf ("      update pot     %12.4f\n", timings[UPDATE_POT_TIME]);
        printf ("         Hartree     %12.4f\n", timings[HARTREE_TIME]);
        printf ("           confine   %12.4f\n", timings[CONFINE_TIME]);
        printf ("           mgrid_vh  %12.4f\n", timings[MGRID_VH_TIME]);
        printf ("*********************************\n\n");

        printf ("      get_Hij        %12.4f\n", timings[GET_Hij_TIME]);
        printf ("---------------------------------\n");
        printf ("        H|psi>       %12.4f\n", timings[H_psi_TIME]);
        printf ("        <psi|psi>    %12.4f\n", timings[ORBIT_DOT_ORBIT_H]);
        printf ("        <kb|psi>     %12.4f\n", timings[get_allkbpsi_TIME]);
        printf ("        get_Hvnl     %12.4f\n", timings[get_Hnl_TIME]);

        printf ("\n");
        printf ("     get_matB        %12.4f\n", timings[GET_MATB_SOFT_TIME]);
        printf ("---------------------------------\n");
        printf ("       <psi|psi>     %12.4f\n", timings[ORBIT_DOT_ORBIT_O]);
        printf ("       matB_qnm      %12.4f\n", timings[matB_qnm_TIME]);

        printf ("\n");
        printf ("     charge mat      %12.4f\n", timings[CHARGE_DEN_MAT_TIME]);
        printf ("---------------------------------\n");
        printf ("       equilibrium   %12.4f\n", timings[EQ_PART_TIME]);
        printf ("       non-equilib   %12.4f\n", timings[NONEQ_PART_TIME]);
        printf ("\n");


        printf ("       equilibrium   %12.4f\n", timings[EQ_PART_TIME]);
        printf ("---------------------------------\n");
        printf ("         global sum  %12.4f\n", timings[MPISUM_EQ_TIME]);
        printf ("         Green_c     %12.4f\n", timings[GREEN_EQ_TIME]);

        printf ("\n");
        printf ("       non-equilib   %12.4f\n", timings[NONEQ_PART_TIME]);
        printf ("---------------------------------\n");
        printf ("         rho_munu    %12.4f\n", timings[RHO_MUNU_TIME]);
        printf ("         Green_c     %12.4f\n", timings[GREEN_NONEQ_TIME]);
        printf ("         inverse     %12.4f\n", timings[matrix_inverse_lr_TIME]);

        fflush (NULL);


    }                           /* end if */
    my_barrier ();
    if (pct.gridpe == 0)
        printf ("\n run done... \n");
}                               /* end write_timings */

/******/
