/************************** SVN Revision Information **************************
 **    $Id: mg_restrict_6.c 1168 2010-12-03 23:53:46Z btan $    **
******************************************************************************/

/****f* QMD-MGDFT/mg_restrict_6.c *****
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
 *   void mg_restrict(REAL *full, REAL *half, int dimx, int dimy, int dimz)
 *   get half dimensioned array in the corse grid from full dimensioned 
 *   array in the fine grid. The returned values are smoothed.
 * INPUTS
 *   full[(dimx+10)*(dimy+10)*(dimz+10)]: array in the fine grid
 *      image value must be there
 *   dimx, dimy, dimz: dimensions of array in the fine grid
 * OUTPUT
 *   half[(dimx/scale)*(dimy/scale)*(dimz/scale)] array in the corse grid
 * PARENTS
 *   mgrid_solv.c
 * CHILDREN
 *   nothing 
 * SOURCE
 */



#include "main.h"
#include <float.h>
#include <math.h>





void mg_restrict_6 (REAL * full, REAL * half, int dimx, int dimy, int dimz , int scale)
{

    int ix, iy, iz;
    int incz, incy, incx, incz2, incy2, incx2, incx3;
    REAL  a0, a1, a2, a3, a4, a5;
    REAL * fulla;
    REAL * fullb;


    incz = 1;
    incy = dimz + 10;
    incx = (dimz + 10) * (dimy + 10);

    incz2 = 1;
    incy2 = dimz / scale ;
    incx2 = (dimz / scale) * (dimy / scale);
 
    incx3 = (dimz + 10) * (dimy / scale);   

    a0 = 3.0/7.0;
    a1 = (1.0-a0)*15.0/20.0;
    a2 = (1.0-a0)*(-6.0)/20.0;
    a3 = (1.0-a0)*1.0/20.0;
    a4 = 0.0;
    a5 = 0.0;

   my_malloc (fulla,(FPX0_GRID / scale)*(FPY0_GRID + 10)*(FPZ0_GRID + 10),REAL);
   my_malloc (fullb,(FPX0_GRID / scale)*(FPY0_GRID / scale)*(FPZ0_GRID + 10),REAL);


        for (ix = 0; ix < dimx / scale; ix++)
        {

            for (iy = 0; iy < dimy + 10; iy++)
            {


                for (iz = 0; iz < dimz + 10; iz++)
                {


                    fulla[ix * incx + iy * incy + iz] =
                               a5* full[(scale*ix+0) * incx + iy * incy + iz] +
                               a4* full[(scale*ix+1) * incx + iy * incy + iz] +
                               a3* full[(scale*ix+2) * incx + iy * incy + iz] +
                               a2* full[(scale*ix+3) * incx + iy * incy + iz] + 
                               a1* full[(scale*ix+4) * incx + iy * incy + iz] +
                               a0* full[(scale*ix+5) * incx + iy * incy + iz] +
                               a1* full[(scale*ix+6) * incx + iy * incy + iz] +
                               a2* full[(scale*ix+7) * incx + iy * incy + iz] +
                               a3* full[(scale*ix+8) * incx + iy * incy + iz] +
                               a4* full[(scale*ix+9) * incx + iy * incy + iz] +
                               a5* full[(scale*ix+10) * incx + iy * incy + iz] ;


                }               /* end for */

            }                   /* end for */

        }                       /* end for */



        for (ix = 0; ix < dimx / scale; ix++)
        {
        
            for (iy = 0; iy < dimy / scale; iy++)
            {
    

                for (iz = 0; iz < dimz + 10; iz++)
                {


                    fullb[ix * incx3 + iy * incy + iz] =
                               a5* fulla[ix * incx + (scale*iy+0) * incy + iz] +
                               a4* fulla[ix * incx + (scale*iy+1) * incy + iz] +
                               a3* fulla[ix * incx + (scale*iy+2) * incy + iz] +
                               a2* fulla[ix * incx + (scale*iy+3) * incy + iz] +
                               a1* fulla[ix * incx + (scale*iy+4) * incy + iz] +
                               a0* fulla[ix * incx + (scale*iy+5) * incy + iz] +
                               a1* fulla[ix * incx + (scale*iy+6) * incy + iz] +
                               a2* fulla[ix * incx + (scale*iy+7) * incy + iz] +
                               a3* fulla[ix * incx + (scale*iy+8) * incy + iz] +
                               a4* fulla[ix * incx + (scale*iy+9) * incy + iz] +
                               a5* fulla[ix * incx + (scale*iy+10) * incy + iz] ;


                }               /* end for */

            }                   /* end for */

        }                       /* end for */



        for (ix = 0; ix < dimx / scale; ix++)
        {
        
            for (iy = 0; iy < dimy / scale; iy++)
            {
    

                for (iz = 0; iz < dimz / scale; iz++)
                {


                    half[ix * incx2 + iy * incy2 + iz] =
                               a5* fullb[ix * incx3 + iy* incy + scale*iz+0 ] +
                               a4* fullb[ix * incx3 + iy* incy + scale*iz+1 ] +
                               a3* fullb[ix * incx3 + iy* incy + scale*iz+2 ] +
                               a2* fullb[ix * incx3 + iy* incy + scale*iz+3 ] +
                               a1* fullb[ix * incx3 + iy* incy + scale*iz+4 ] +
                               a0* fullb[ix * incx3 + iy* incy + scale*iz+5 ] +
                               a1* fullb[ix * incx3 + iy* incy + scale*iz+6 ] +
                               a2* fullb[ix * incx3 + iy* incy + scale*iz+7 ] +
                               a3* fullb[ix * incx3 + iy* incy + scale*iz+8 ] +
                               a4* fullb[ix * incx3 + iy* incy + scale*iz+9 ] +
                               a5* fullb[ix * incx3 + iy* incy + scale*iz+10 ] ;


                }               /* end for */

            }                   /* end for */

        }                       /* end for */


        my_free (fulla);
        my_free (fullb);




}                               /* end mg_restrict */


/******/
