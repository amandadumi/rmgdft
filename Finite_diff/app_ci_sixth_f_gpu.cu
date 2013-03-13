#include "make_conf.h"
#if GPU_ENABLED
#include "fixed_dims.h"
#ifdef FD_XSIZE
__global__ void app_cil_sixth_f_kernel(const float * __restrict__ psi, 
                                                float *b, 
                                                const int dimx,
                                                const int dimy,
                                                const int dimz,
                                                const float cc,
                                                const float fcx,
                                                const float ecxz,
                                                const float cor,
                                                const float fc2x,
                                                const float tcx)
{

    __shared__ float slice[FIXED_YDIM/2 + 4][FIXED_ZDIM + 4];
    float accp_b2, accp_b1, accp, accp_a1, accp_a2;
    float accm_b2, accm_b1, accm, accm_a1;
    float acc, acc_a1, acc_a2;

    // iz and iy map to the x and y coordinates of the thread
    // within a block
    int iz = blockIdx.x * blockDim.x + threadIdx.x + 2;
    int iy = blockIdx.y * blockDim.y + threadIdx.y + 2;

    // thread z-index into shared memory tile
    int tz = threadIdx.x + 2;
    // thread y-index into shared memory tile
    int ty = threadIdx.y + 2;
    int ix=0;

    int incx = (dimz + 4) * (dimy + 4);
    int incy = dimz + 4;
    int incxr = dimz * dimy;
    int incyr = dimz;

    int ixs, ixps;
    int iys, iyms, iyps, iymms, iypps;
    int izs, izms, izps, izmms, izpps;

    ixs = ix * incx;
    ixps = (ix + 1) * incx;

    iys = iy * incy;
    iyms = (iy - 1) * incy;
    iyps = (iy + 1) * incy;
    iymms = (iy - 2) * incy;
    iypps = (iy + 2) * incy;
    izs = iz;
    izms = (iz - 1);
    izps = (iz + 1);
    izmms = (iz - 2);
    izpps = (iz + 2);

    accp_b1 = 0.0;

    accp_a1 =
            fc2x * psi[izs + iys + ixs] +
            tcx * psi[izms + iys + ixs] +
            tcx * psi[izps + iys + ixs] +
            tcx * psi[izs + iyps + ixs] +
            tcx * psi[izs + iyms + ixs];

    accm =
            fcx * psi[izs + iys + ixs] +
            ecxz * psi[izms + iys + ixs] +
            ecxz * psi[izps + iys + ixs] +
            ecxz * psi[izs + iyms + ixs] +
            ecxz * psi[izs + iyps + ixs] +
            cor * psi[izms + iyms + ixs] +
            cor * psi[izps + iyms + ixs] +
            cor * psi[izms + iyps + ixs] +
            cor * psi[izps + iyps + ixs] +
            tcx * psi[izpps + iys + ixs] +
            tcx * psi[izmms + iys + ixs] +
            tcx * psi[izs + iypps + ixs] +
            tcx * psi[izs + iymms + ixs];


    acc_a1 = cc * psi[izs + iys + ixs] +
            fcx * psi[izms + iys + ixs] +
            fcx * psi[izps + iys + ixs] +
            fcx * psi[izs + iyms + ixs] +
            fcx * psi[izs + iyps + ixs] +
            ecxz * psi[izms + iyms + ixs] +
            ecxz * psi[izms + iyps + ixs] +
            ecxz * psi[izps + iyms + ixs] +
            ecxz * psi[izps + iyps + ixs] +
            fc2x * psi[izmms + iys + ixs] +
            fc2x * psi[izpps + iys + ixs] +
            fc2x * psi[izs + iymms + ixs] +
            fc2x * psi[izs + iypps + ixs] +
            tcx * psi[izps + iypps + ixs] +
            tcx * psi[izps + iymms + ixs] +
            tcx * psi[izms + iypps + ixs] +
            tcx * psi[izms + iymms + ixs] +
            tcx * psi[izpps + iyps + ixs] +
            tcx * psi[izmms + iyps + ixs] +
            tcx * psi[izpps + iyms + ixs] +
            tcx * psi[izmms + iyms + ixs];

    accp_a2 =
            fc2x * psi[izs + iys + ixps] +
            tcx * psi[izms + iys + ixps] +
            tcx * psi[izps + iys + ixps] +
            tcx * psi[izs + iyps + ixps] +
            tcx * psi[izs + iyms + ixps];

    accm_a1 =
            fcx * psi[izs + iys + ixps] +
            ecxz * psi[izms + iys + ixps] +
            ecxz * psi[izps + iys + ixps] +
            ecxz * psi[izs + iyms + ixps] +
            ecxz * psi[izs + iyps + ixps] +
            cor * psi[izms + iyms + ixps] +
            cor * psi[izps + iyms + ixps] +
            cor * psi[izms + iyps + ixps] +
            cor * psi[izps + iyps + ixps] +
            tcx * psi[izpps + iys + ixps] +
            tcx * psi[izmms + iys + ixps] +
            tcx * psi[izs + iypps + ixps] +
            tcx * psi[izs + iymms + ixps];

    acc_a2 = cc * psi[izs + iys + ixps] +
            fcx * psi[izms + iys + ixps] +
            fcx * psi[izps + iys + ixps] +
            fcx * psi[izs + iyms + ixps] +
            fcx * psi[izs + iyps + ixps] +
            ecxz * psi[izms + iyms + ixps] +
            ecxz * psi[izms + iyps + ixps] +
            ecxz * psi[izps + iyms + ixps] +
            ecxz * psi[izps + iyps + ixps] +
            fc2x * psi[izmms + iys + ixps] +
            fc2x * psi[izpps + iys + ixps] +
            fc2x * psi[izs + iymms + ixps] +
            fc2x * psi[izs + iypps + ixps] +
            tcx * psi[izps + iypps + ixps] +
            tcx * psi[izps + iymms + ixps] +
            tcx * psi[izms + iypps + ixps] +
            tcx * psi[izms + iymms + ixps] +
            tcx * psi[izpps + iyps + ixps] +
            tcx * psi[izmms + iyps + ixps] +
            tcx * psi[izpps + iyms + ixps] +
            tcx * psi[izmms + iyms + ixps];


    for (ix = 0; ix < dimx + 2; ix++)
    {

        // Advance the slice partial sums
        accp_b2 = accp_b1;
        accp_b1 = accp;
        accp = accp_a1;
        accp_a1 = accp_a2;

        accm_b2 = accm_b1;
        accm_b1 = accm;
        accm = accm_a1;

        acc = acc_a1;
        acc_a1 = acc_a2;

        __syncthreads();
        // Update the data slice in shared memory
        if(threadIdx.x < 2) {
            slice[ty][threadIdx.x] =
                            psi[(ix + 2)*incx + iy*incy + (threadIdx.x + blockIdx.x*blockDim.x)];
            slice[ty][threadIdx.x + blockDim.x + 2] =
                            psi[(ix + 2)*incx + iy*incy + (threadIdx.x + blockDim.x + 2 + blockIdx.x*blockDim.x)];
        }
        if(threadIdx.y < 2) {
            slice[threadIdx.y][tz] =
                            psi[(ix + 2)*incx + (threadIdx.y + blockIdx.y*blockDim.y)*incy + iz];
            slice[threadIdx.y + blockDim.y + 2][tz] =
                            psi[(ix + 2)*incx + (threadIdx.y + blockDim.y + 2 + blockIdx.y*blockDim.y)*incy + iz];
        }

        if((threadIdx.x == 2) && (threadIdx.y == 2)) {
            slice[0][0] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y)*incy + blockIdx.x*blockDim.x];
            slice[1][0] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y + 1)*incy + blockIdx.x*blockDim.x];
            slice[0][1] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y)*incy + blockIdx.x*blockDim.x + 1];
            slice[1][1] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y + 1)*incy + blockIdx.x*blockDim.x + 1];
        }
        if((threadIdx.x == 2) && (threadIdx.y == 3)) {
            slice[0][blockDim.x + 2] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y)*incy + blockIdx.x*blockDim.x + blockDim.x + 2];
            slice[1][blockDim.x + 3] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y + 1)*incy + blockIdx.x*blockDim.x + blockDim.x + 3];
            slice[1][blockDim.x + 2] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y + 1)*incy + blockIdx.x*blockDim.x + blockDim.x + 2];
            slice[0][blockDim.x + 3] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y)*incy + blockIdx.x*blockDim.x + blockDim.x + 3];
        }

       if((threadIdx.x == 3) && (threadIdx.y == 2)) {
            slice[blockDim.y + 2][0] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y + blockDim.y + 2)*incy + blockIdx.x*blockDim.x];
            slice[blockDim.y + 3][0] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y + blockDim.y + 3)*incy + blockIdx.x*blockDim.x];
            slice[blockDim.y + 2][1] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y + blockDim.y + 2)*incy + blockIdx.x*blockDim.x + 1];
            slice[blockDim.y + 3][1] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y + blockDim.y + 3)*incy + blockIdx.x*blockDim.x + 1];
        }

        if((threadIdx.x == 3) && (threadIdx.y == 3)) {
            slice[blockDim.y + 2][blockDim.x + 2] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y + blockDim.y + 2)*incy + blockIdx.x*blockDim.x + blockDim.x + 2];
            slice[blockDim.y + 2][blockDim.x + 3] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y + blockDim.y + 2)*incy + blockIdx.x*blockDim.x + blockDim.x + 3];
            slice[blockDim.y + 3][blockDim.x + 2] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y + blockDim.y + 3)*incy + blockIdx.x*blockDim.x + blockDim.x + 2];
            slice[blockDim.y + 3][blockDim.x + 3] =
                            psi[(ix + 2)*incx + (blockIdx.y*blockDim.y + blockDim.y + 3)*incy + blockIdx.x*blockDim.x + blockDim.x + 3];
        }

        // Put the xy tile for the leading x (+2) index into shared memory
        slice[ty][tz] = psi[(ix + 2)*incx + iy*incy + iz];

        __syncthreads();

        acc_a2 = cc * slice[ty][tz] +
            fcx *  (slice[ty][(tz - 1)] +
                    slice[ty][(tz + 1)] +
                    slice[(ty - 1)][tz] +
                    slice[(ty + 1)][tz]) +
            ecxz * (slice[(ty - 1)][(tz - 1)] +
                    slice[(ty + 1)][(tz - 1)] +
                    slice[(ty - 1)][(tz + 1)] +
                    slice[(ty + 1)][(tz + 1)]) +
            fc2x * (slice[ty][(tz - 2)] +
                    slice[ty][(tz + 2)] +
                    slice[(ty - 2)][tz] +
                    slice[(ty + 2)][tz]) +
            tcx *  (slice[(ty + 2)][(tz + 1)] +
                    slice[(ty - 2)][(tz + 1)] +
                    slice[(ty + 2)][(tz - 1)] +
                    slice[(ty - 2)][(tz - 1)] +
                    slice[(ty + 1)][(tz + 2)] +
                    slice[(ty + 1)][(tz - 2)] +
                    slice[(ty - 1)][(tz + 2)] +
                    slice[(ty - 1)][(tz - 2)]);

        accm_a1 =
            fcx * slice[ty][tz] +
            ecxz * (slice[ty][(tz - 1)] +
                    slice[ty][(tz + 1)] +
                    slice[(ty - 1)][tz] +
                    slice[(ty + 1)][tz]) +
            cor *  (slice[(ty - 1)][(tz - 1)] +
                    slice[(ty - 1)][(tz + 1)] +
                    slice[(ty + 1)][(tz - 1)] +
                    slice[(ty + 1)][(tz + 1)]) +
            tcx *  (slice[ty][(tz + 2)] +
                    slice[ty][(tz - 2)] +
                    slice[(ty + 2)][tz] +
                    slice[(ty - 2)][tz]);

        accp_a2 =
            fc2x * slice[ty][tz] +
            tcx *  (slice[ty][(tz - 1)] +
                    slice[ty][(tz + 1)] +
                    slice[(ty + 1)][tz] +
                    slice[(ty - 1)][tz]);

        // Write back the results
        if(ix >= 2) {
            b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] = acc +

                                                            accm_b2 +
                                                            accm +

                                                            accp_b2 +
                                                            accp_a2;
        }

    }                   /* end for */

}

__global__ void app_cir_sixth_f_kernel(const float * __restrict__ psi, 
                                                float *b, 
                                                const int dimx,
                                                const int dimy,
                                                const int dimz,
                                                const float c000,
                                                const float c100,
                                                const float c110,
                                                const float c200)
{


    // iz and iy map to the x and y coordinates of the thread
    // withing a block
    int iz = blockIdx.x * blockDim.x + threadIdx.x + 2;
    int iy = blockIdx.y * blockDim.y + threadIdx.y + 2;

    int ix=0;

    int incx = (dimz + 4) * (dimy + 4);
    int incy = dimz + 4;
    int incxr = dimz * dimy;
    int incyr = dimz;

    int ixs, ixps, ixms, ixmms, ixpps;
    int iys, iyms, iyps, iymms, iypps;
    float sum;


    iys = iy * incy;
    iyms = (iy - 1) * incy;
    iyps = (iy + 1) * incy;
    iymms = (iy - 2) * incy;
    iypps = (iy + 2) * incy;

    for (ix = 2; ix < dimx + 2; ix++)
    {
		ixs = ix * incx;
		ixms = (ix - 1) * incx;
		ixps = (ix + 1) * incx;
		ixmms = (ix - 2) * incx;
		ixpps = (ix + 2) * incx;

                sum = c000 * psi[ixs + iys + iz] +
                      c100 * (psi[ixs + iys + (iz - 1)] +
                              psi[ixs + iys + (iz + 1)] +
                              psi[ixms + iys + iz] +
                              psi[ixps + iys + iz] +
                              psi[ixs + iyms + iz] +
                              psi[ixs + iyps + iz]);

                sum += c110 * (psi[ixps + iyps + iz] +
                               psi[ixps + iyms + iz] +
                               psi[ixms + iyps + iz] +
                               psi[ixms + iyms + iz] +
                               psi[ixps + iys + (iz + 1)] +
                               psi[ixps + iys + (iz - 1)] +
                               psi[ixms + iys + (iz + 1)] +
                               psi[ixms + iys + (iz - 1)] +
                               psi[ixs + iyps + (iz + 1)] +
                               psi[ixs + iyps + (iz - 1)] +
                               psi[ixs + iyms + (iz + 1)] + 
                               psi[ixs + iyms + (iz - 1)]);

                b[(ix - 2) * incxr + (iy - 2) * incyr + (iz - 2)] = sum +
                    c200 * (psi[ixs + iys + (iz - 2)] +
                            psi[ixs + iys + (iz + 2)] +
                            psi[ixmms + iys + iz] +
                            psi[ixpps + iys + iz] +
                            psi[ixs + iymms + iz] + 
                            psi[ixs + iypps + iz]);

        
    }                   /* end for */


}




// C wrapper functions that call the cuda kernels above
extern "C" void app_cil_sixth_f_gpu(const float *psi, 
                                                float *b, 
                                                const int dimx,
                                                const int dimy,
                                                const int dimz,
                                                const double gridhx,
                                                const double gridhy,
                                                const double gridhz,
                                                const double xside,
                                                const double yside,
                                                const double zside,
                                                cudaStream_t cstream)
{


  dim3 Grid, Block;
  Grid.x = 2;
  Grid.y = 2;
  Block.x = dimz/Grid.x;
  Block.y = dimy/Grid.y;
  double ihx = 1.0 / (gridhx * gridhx * xside * xside);
  double ihy = 1.0 / (gridhy * gridhy * yside * yside);
  double ihz = 1.0 / (gridhz * gridhz * zside * zside);
  double cc = (-116.0 / 90.0) * (ihx + ihy + ihz);
  double fcx = (31.0 / 232.0) * cc + (49.0 / 60.0) * ihx;
  double ecxz = (-31.0 / 464.0) * cc - (1.0 / 10.0) * ihz;
  double cor = (1.0 / 144.0) * (ihx + ihy + ihz);
  double fc2x = (1.0 / 120.0) * (ihy + ihz);
  double tcx = (-1.0 / 240.0) * ihx;

  app_cil_sixth_f_kernel<<<Grid, Block, 0, cstream>>>(
                                                   psi,
                                                   b,
                                                   dimx,    
                                                   dimy,    
                                                   dimz,    
                                                   (float)cc,
                                                   (float)fcx,
                                                   (float)ecxz,
                                                   (float)cor,
                                                   (float)fc2x,
                                                   (float)tcx);
}

extern "C" void app_cir_sixth_f_gpu(const float *psi, 
                                                float *b, 
                                                const int dimx,
                                                const int dimy,
                                                const int dimz,
                                                cudaStream_t cstream)
{

  dim3 Grid, Block;
  Grid.x = 2;
  Grid.y = 2;
  Block.x = dimz/Grid.x;
  Block.y = dimy/Grid.y;

  double c000 = 61.0 / 120.0;
  double c100 = 13.0 / 180.0;
  double c110 = 1.0 / 144.0;
  double c200 = -1.0 / 240.0;

  app_cir_sixth_f_kernel<<<Grid, Block, 0, cstream>>>(
                                                   psi,
                                                   b,
                                                   dimx,    
                                                   dimy,    
                                                   dimz,
                                                   (float)c000,
                                                   (float)c100,
                                                   (float)c110,
                                                   (float)c200);
}
#endif
#endif
