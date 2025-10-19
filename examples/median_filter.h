#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MEDIAN_FILTER_SIZE 17

typedef struct {
	uint16_t window[MEDIAN_FILTER_SIZE]; // Circular buffer for incoming samples
	uint16_t sorted[MEDIAN_FILTER_SIZE]; // A sorted copy of the window
	int index; // Current position in the circular buffer
	int count; // Number of samples in the window (for initialization)
} median_filter_t;

void median_filter_init(median_filter_t *filt);
uint16_t median_filter_update(median_filter_t *filt, uint16_t new_sample);

#ifdef __cplusplus
}
#endif

#endif /* MEDIAN_FILTER_H */
