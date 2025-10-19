#include "high_reference_filter.h"
#include <string.h>

/**
 * @brief Initializes the high-pass filter state.
 *
 * Ultra-optimized version using cascaded buffers with single index.
 *
 * @param filt Pointer to the filter state structure.
 */
void high_reference_filter_init(RCfilter_high_t *filt)
{
	if (!filt)
	{
		return;
	}
	memset(filt->buffer_32, 0, sizeof(filt->buffer_32));
	memset(filt->buffer_64, 0, sizeof(filt->buffer_64));
	memset(filt->buffer_96, 0, sizeof(filt->buffer_96));
	memset(filt->y, 0, sizeof(filt->y));
	filt->idx = 0;
}

/**
 * @brief Updates the high-pass filter with a new input sample.
 *
 * Ultra-optimized cascaded buffer implementation with single synchronized index.
 *
 * Memory: 96 uint32_t (384 bytes) vs original 97 uint32_t (388 bytes) = 4 bytes saved
 * Performance: Single index update, better cache locality, fewer modulo operations
 *
 * Filter equation:
 * y[n] = 2*y[n-1] - y[n-2] + x[n] - x[n-32] - x[n-64] + x[n-96]
 *
 * Cascade strategy:
 * - buffer_32: Holds x[n-1] to x[n-32]
 * - buffer_64: Holds x[n-33] to x[n-64]
 * - buffer_96: Holds x[n-65] to x[n-96]
 *
 * Data flows: input → buffer_32[idx] → buffer_64[idx] → buffer_96[idx]
 *
 * @param filt Pointer to the filter state structure.
 * @param input The new input sample, x[n].
 * @return The calculated output sample, y[n].
 */
uint16_t high_reference_filter_update(RCfilter_high_t *filt, uint16_t input)
{
	if (!filt)
	{
		return 0;
	}

	// Read the values we need BEFORE they get overwritten
	// x[n-32] is in buffer_32 at current index
	uint32_t x_32 = filt->buffer_32[filt->idx];

	// x[n-64] is in buffer_64 at current index
	uint32_t x_64 = filt->buffer_64[filt->idx];

	// x[n-96] is in buffer_96 at current index
	uint32_t x_96 = filt->buffer_96[filt->idx];

	// Cascade the data through buffers (oldest gets overwritten)
	// buffer_96 receives what was in buffer_64 (x[n-64] becomes x[n-96] after 32 more samples)
	filt->buffer_96[filt->idx] = x_64;

	// buffer_64 receives what was in buffer_32 (x[n-32] becomes x[n-64] after 32 more samples)
	filt->buffer_64[filt->idx] = x_32;

	// buffer_32 receives the new input (x[n] becomes x[n-32] after 32 samples)
	filt->buffer_32[filt->idx] = input;

	// Retrieve output history
	uint32_t y_1 = filt->y[0]; // y[n-1]
	uint32_t y_2 = filt->y[1]; // y[n-2]

	// Calculate new output: y[n] = 2*y[n-1] - y[n-2] + x[n] - x[n-32] - x[n-64] + x[n-96]
	uint32_t current_output = (y_1 * 2) - y_2 + input - x_32 - x_64 + x_96;

	// Update output history
	filt->y[1] = filt->y[0];	 // Shift: y[n-1] → y[n-2]
	filt->y[0] = current_output; // Store: y[n] → y[n-1]

	// Advance synchronized circular buffer index (single update for all three buffers!)
	filt->idx = (filt->idx + 1) % 32;

	// Apply scaling factor
	current_output = (current_output / 2048);

	return (uint16_t)current_output;
}
