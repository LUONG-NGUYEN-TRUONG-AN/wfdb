#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

#include "project.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEDIAN_FILTER_SIZE 17

typedef struct {
    WFDB_Time window[MEDIAN_FILTER_SIZE]; // Circular buffer for incoming samples
    WFDB_Time sorted[MEDIAN_FILTER_SIZE]; // A sorted copy of the window
    int index;                            // Current position in the circular buffer
    int count;                            // Number of samples in the window (for initialization)
} median_filter_t;

void median_filter_init(median_filter_t *filt);
int32_t median_filter_update(median_filter_t *filt, WFDB_Time new_sample);


#ifdef __cplusplus
}
#endif


#endif /* MEDIAN_FILTER_H */
