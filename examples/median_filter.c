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

/**
 * Initialize the median filter state
 */
void median_filter_init(median_filter_t *filt) {
    if (!filt) return;
    memset(filt->window, 0, sizeof(filt->window));
    memset(filt->sorted, 0, sizeof(filt->sorted));
    filt->index = 0;
    filt->count = 0;
}

/**
 * Update the median filter with a new sample and return the new median.
 * @param filt: Pointer to the filter state structure
 * @param new_sample: The current input sample
 * @return: The new median value
 */
int32_t median_filter_update(median_filter_t *filt, WFDB_Time new_sample) {
    if (!filt) return 0;

    // Add the new sample to the circular window buffer
    filt->window[filt->index] = new_sample;
    filt->index = (filt->index + 1) % MEDIAN_FILTER_SIZE;

    // Keep track of how many samples we have until the window is full
    if (filt->count < MEDIAN_FILTER_SIZE) {
        filt->count++;
    }

    // To find the median, we work on a sorted copy of the window.
    // This is more efficient than complex insertion/deletion algorithms for small N.
    memcpy(filt->sorted, filt->window, sizeof(filt->window));
    qsort(filt->sorted, filt->count, sizeof(WFDB_Time), icmp);

    // The median is the middle element of the sorted array.
    // If the window is not yet full, we take the middle of the current samples.
    return filt->sorted[filt->count / 2];
}
