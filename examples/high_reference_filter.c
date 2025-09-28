#include "high_reference_filter.h"
#include <string.h>

/**
 * @brief Initializes the high-pass filter state.
 *
 * @param filt Pointer to the filter state structure.
 */
void high_reference_filter_init(RCfilter_high_t *filt) {
	if (!filt) {
		return;
	}
	memset(filt->x, 0, sizeof(filt->x));
	memset(filt->y, 0, sizeof(filt->y));
	filt->index = 0;
}

/**
 * @brief Updates the high-pass filter with a new input sample.
 *
 * Implements the filter equation:
 * y[n] = 2*y[n-1] - y[n-2] + x[n] - x[n-32] - x[n-64] + x[n-96]
 *
 * @param filt Pointer to the filter state structure.
 * @param input The new input sample, x[n].
 * @return The calculated output sample, y[n].
 */
int32_t high_reference_filter_update(RCfilter_high_t *filt, int32_t input) {
	if (!filt) {
		return 0;
	}

	// Store the new input x[n] in the circular buffer, overwriting the oldest
	// sample x[n-97]
	filt->x[filt->index] = input;

	// Retrieve historical input values from the circular buffer
	// x[n] is the current 'input'
	int32_t x_n_32 = filt->x[(filt->index + HIGH_REFERENCE_FILTER_ORDER - 32) %
							 HIGH_REFERENCE_FILTER_ORDER];
	int32_t x_n_64 = filt->x[(filt->index + HIGH_REFERENCE_FILTER_ORDER - 64) %
							 HIGH_REFERENCE_FILTER_ORDER];
	int32_t x_n_96 = filt->x[(filt->index + HIGH_REFERENCE_FILTER_ORDER - 96) %
							 HIGH_REFERENCE_FILTER_ORDER];

	// Retrieve historical output values
	int32_t y_n_1 = filt->y[0]; // y[n-1]
	int32_t y_n_2 = filt->y[1]; // y[n-2]

	// Calculate the new output y[n]
	// y[n] = 2*y[n-1] - y[n-2] + x[n] - x[n-32] - x[n-64] + x[n-96]
	int32_t y_n = (y_n_1 * 2) - y_n_2 + input - x_n_32 - x_n_64 + x_n_96;

	// Update the output history for the next iteration
	filt->y[1] = filt->y[0]; // Old y[n-1] becomes new y[n-2]
	filt->y[0] = y_n;		 // New y[n] becomes new y[n-1]

	// Advance the circular buffer index
	filt->index = (filt->index + 1) % HIGH_REFERENCE_FILTER_ORDER;

	return (y_n / 2048);
}
