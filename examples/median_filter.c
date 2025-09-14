#include "median_filter.h"
#include <stdlib.h>
#include <assert.h>

#define K 17
#define K2 ((K - 1) / 2)

int icmp(const void *a, const void *b)
{
    WFDB_Time x = *(const WFDB_Time *)a;
    WFDB_Time y = *(const WFDB_Time *)b;
    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0; // For ascending order
}

void median_filter(const WFDB_Time *x, int32_t *output, int32_t n)
{
    WFDB_Time window[K];

    for (int i = 0; i < n; i++)
    {
        // Fill the window with extended boundaries
        for (int j = 0; j < K; j++)
        {
            int idx = i + j - K2;

            if (idx < 0)
            {
                window[j] = x[0]; // Extend with first element
            }
            else if (idx >= n)
            {
                window[j] = x[n - 1]; // Extend with last element
            }
            else
            {
                window[j] = x[idx];
            }
        }

        // Sort the window
        qsort(window, K, sizeof(WFDB_Time), icmp);

        // Assign the median to output
        output[i] = window[K2];
    }
}
