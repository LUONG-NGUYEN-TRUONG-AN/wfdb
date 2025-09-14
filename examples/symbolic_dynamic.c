#include "symbolic_dynamic.h"
#include <sys/stat.h>
#include <errno.h>

#define X_DELAY 63  // Delay for x array
#define XL_DELAY 47 // Delay for xl array
// Helper function to ensure the "gen" directory exists
void ensure_gen_directory(void)
{
    struct stat st = {0};
    if (stat("gen", &st) == -1)
    {
        if (mkdir("gen", 0755) == -1)
        {
            fprintf(stderr, "Warning: Could not create 'gen' directory: %s\n", strerror(errno));
        }
    }
}

void symbolic_dynamic(const WFDB_Time *x, const int32_t *xl, const int32_t *xh, uint8_t *sy, WFDB_Time len)
{
    int32_t DeltaRR[len];
    for (WFDB_Time i = 0; i < len; i++)
    {
        int32_t x_delayed, xl_delayed;

        // Get delayed x value with bounds checking
        if (i >= X_DELAY)
        {
            x_delayed = x[i - X_DELAY];
        }
        else
        {
            x_delayed = x[0]; // Use first value for early samples
        }

        // Get delayed xl value with bounds checking
        if (i >= XL_DELAY)
        {
            xl_delayed = xl[i - XL_DELAY];
        }
        else
        {
            xl_delayed = xl[0]; // Use first value for early samples
        }

        // Calculate delta with proper delay indexing
        DeltaRR[i] = x_delayed - xl_delayed;
    }
    for (uint32_t i = 0; i < len; i++)
    {
        int32_t thres1 = xh[i] >> 4; // xh[i] / 16 (16 = 2^4)
        int32_t thres2 = xh[i] >> 3; // xh[i] / 8 (8 = 2^3)
        int32_t thres3 = thres1 + thres2;
        int32_t thres4 = xh[i] >> 2; // xh[i] / 4 (4 = 2^2)
        int32_t thres5 = thres4 + thres1;
        
        if (DeltaRR[i] < -thres4)
        {
            sy[i] = 0;
        }
        else if (DeltaRR[i] < -thres3)
        {
            sy[i] = 1;
        }
        else if (DeltaRR[i] < -thres2)
        {
            sy[i] = 2;
        }
        else if (DeltaRR[i] < -thres1)
        {
            sy[i] = 3;
        }
        else if (DeltaRR[i] < thres1)
        {
            sy[i] = 4;
        }
        else if (DeltaRR[i] < thres2)
        {
            sy[i] = 5;
        }
        else if (DeltaRR[i] < thres3)
        {
            sy[i] = 6;
        }
        else if (DeltaRR[i] < thres4)
        {
            sy[i] = 7;
        }
        else if (DeltaRR[i] < thres5)
        {
            sy[i] = 8;
        }
        else
        {
            sy[i] = 9;
        }
    }
}
