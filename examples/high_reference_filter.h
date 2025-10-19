#ifndef HIGH_REFERENCE_FILTER_H
#define HIGH_REFERENCE_FILTER_H

#include "project.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define HIGH_REFERENCE_FILTER_ORDER 96 // Maximum delay needed

	// Ultra-optimized structure: Only store what we actually need!
	// Uses cascaded shift approach - stores only required delay samples
	typedef struct
	{
		uint32_t buffer_32[32]; // Circular buffer for delays 1-32
		uint32_t buffer_64[32]; // Circular buffer for delays 33-64
		uint32_t buffer_96[32]; // Circular buffer for delays 65-96
		uint32_t y[2];			// Output history: y[n-1] and y[n-2]
		uint8_t idx;			// Single index for all buffers (synchronized)
	} RCfilter_high_t;

	// Memory usage: 96 uint32_t + 2 uint32_t + 1 uint8_t = 393 bytes
	// vs Original: 97 uint32_t + 2 uint32_t + 1 uint8_t = 397 bytes
	// Savings: 4 bytes + improved cache locality

	/**
	 * @brief Initialize the high reference filter
	 *
	 * @param filt: Filter parameter
	 */
	void high_reference_filter_init(RCfilter_high_t *filt);

	/**
	 * High reference filter implementation: y[n] = (y[n-1] * 2) - y[n-2] + x[n] -
	 * x[n-32] - x[n-64] - x[n-96] with 47-sample output delay
	 * @param input: Input signal array
	 * @param output: Output signal array
	 * @param length: Signal length
	 */
	uint16_t high_reference_filter_update(RCfilter_high_t *filt, uint16_t input);

#ifdef __cplusplus
}
#endif

#endif /* HIGH_REFERENCE_FILTER_H */
