/************************** SVN Revision Information **************************
 **    $Id$    **
******************************************************************************/

/****f* QMD-MGDFT/fastrlx.c *****
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
 *   void fastrlx(STATE *states, REAL *vxc, REAL *vh, REAL *vnuc,
 *                REAL *rho, REAL *rhocore, REAL *rhoc)
 *   drive routine for fast relax.
 * INPUTS
 *   states: all wave functions (see main.h)
 *   vxc: exchange-correlation potential
 *   vh:  Hartree potential
 *   vnuc: pseudopotential
 *   rho:  total charge density
 *   rhocore: charge density of core electrons, only useful when we 
 *            include non-linear core correction for pseudopotential.
 *   rhoc:    compensating charge density
 * OUTPUT
 *   all the above inputs are updated
 * PARENTS
 *   main.c
 * CHILDREN
 *   quench.c to_crystal.c to_cartesian.c init_nuc.c get_nlop.c scf.c
 *   get_te.c force.c rmg_fastrelax.c
 * SOURCE
 */

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "main.h"



/* Local function prototypes */
void rmg_fastrelax (void);
void movie (FILE *);



void fastrlx (STATE * states, REAL * vxc, REAL * vh, REAL * vnuc,
              REAL * rho, REAL * rhocore, REAL * rhoc)
{

    int iion;
    int CONV_FORCE, MAX_STEPS;
    int DONE, rlx_steps = 0;
    FILE *mfp = NULL;
    FILE *xbsfp1 = NULL;
    char xbs_filename[60];

    /* if ( ct.override_atoms == 1 ) quench(states, vxc, vh, vnuc, rho, rhocore, rhoc); */

    /* open movie file and output initial frame */
    if ((ct.rmvmovie) && (ct.max_md_steps > 1 && pct.gridpe == 0))
    {
        my_fopen (mfp, "traj.rmv", "w");
        if (setvbuf (mfp, (char *) NULL, _IOFBF, 4096 * 16) != 0)
            printf ("\n Warning: cant allocate movie io buffer size\n");

        movie (mfp);
    }

    /* open XBS movie file */
    if ((ct.xbsmovie) && (ct.max_md_steps > 1 && pct.gridpe == 0))
    {

        strcpy (xbs_filename, "traj");
        xbsfp1 = open_xbs_movie (xbs_filename);
    }


	/* quench the electrons and calculate forces */
	quench (states, vxc, vh, vnuc, rho, rhocore, rhoc);

    /* ---------- begin relax loop --------- */

    DONE = (ct.max_md_steps < 1 );

    while (!DONE)
    {

		rlx_steps++;

        if (pct.gridpe == 0)
            printf ("\nfastrlx: ---------- [rlx: %d/%d] ----------\n", rlx_steps, ct.max_md_steps);

        /* not done yet ? => move atoms */
		/* move the ions */
		rmg_fastrelax ();

		/* ct.md_steps measures the number of updates to the atomic positions */
		ct.md_steps++;

		/* update items that change when the ionic coordinates change */
		init_nuc (vnuc, rhoc, rhocore);
		get_nlop ();
		get_weight ();
		get_QI ();
		get_qqq ();


		/* quench the electrons and calculate forces */
		quench (states, vxc, vh, vnuc, rho, rhocore, rhoc);


		/* save data to file for future restart */
		if (ct.checkpoint)
			if ( ct.md_steps % ct.checkpoint == 0 )
				write_data (ct.outfile, vh, rho, vxc, states);


		/* check force convergence */
		CONV_FORCE = TRUE;
		for (iion = 0; iion < ct.num_ions; iion++)
		{
			if (ct.ions[iion].movable)
			{
				REAL *fp;
				fp = ct.ions[iion].force[ct.fpt[0]];
				CONV_FORCE &=
					((fp[0] * fp[0] + fp[1] * fp[1] + fp[2] * fp[2]) < ct.thr_frc * ct.thr_frc);
			}
		}

		/* check for max relax steps */
		MAX_STEPS = (rlx_steps >= ct.max_md_steps) || ( ct.md_steps > ct.max_md_steps);

		/* done if forces converged or reached limit of md steps */
		DONE = (CONV_FORCE || MAX_STEPS);

	}
	/* ---------- end relax loop --------- */

	if (ct.max_md_steps > 0)
	{

		printf ("\n");
		progress_tag ();

		if (CONV_FORCE)
			printf ("force convergence has been achieved. stopping ...\n");
		else
			printf ("force convergence has NOT been achieved. stopping (max number of relax steps reached) ...\n");

	}



	/*Write out final data */
	write_data (ct.outfile, vh, rho, vxc, states);


}                               /* end fastrlx */


/******/
