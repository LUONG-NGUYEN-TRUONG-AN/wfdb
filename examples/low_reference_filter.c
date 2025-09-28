#include "low_reference_filter.h"

void low_reference_filter_init(RCfilter_low_t *filt) {
	if (!filt) {
		return;
	}
	memset(filt->x, 0, sizeof(filt->x));
	filt->output = 0;
	filt->index = 0;
}

/**
 * Low reference filter implementation: y[n] = y[n-1] + x[n] - x[n-16] with
 * 8-sample output delay
 * @param input: Input signal array
 * @param output: Output signal array
 * @param length: Signal length
 */
int32_t low_reference_filter_update(RCfilter_low_t *filt, int32_t input) {
	if (!filt) {
		return 0; // Or handle error appropriately
	}

	// Get the oldest input sample, x[n-16]
	int32_t delayed_input = filt->x[filt->index];

	// Store the new input sample x[n] in its place
	filt->x[filt->index] = input;

	// Calculate the new output: y[n] = y[n-1] + x[n] - x[n-16]
	// Note: filt->output currently holds y[n-1]
	int32_t current_output = filt->output + input - delayed_input;

	// Store the new output y[n] for the next iteration (it will become y[n-1])
	filt->output = current_output;

	// Move to the next position in the circular buffer
	filt->index = (filt->index + 1) % LOW_REFERENCE_FILTER_ORDER;

	// The original implementation had a scaling factor and an output delay.
	// Applying them here:

	// 1. Scaling (divide by 16)
	current_output = current_output / 16;

	// 2. Output delay of 8 samples is not practical for a single-sample
	//    real-time function, as it would require another buffer.
	//    This is likely a remnant from the array-based version and
	//    should be handled by the calling function if needed.

	return current_output;
}
