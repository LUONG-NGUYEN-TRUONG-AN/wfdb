#ifndef HIGH_REFERENCE_FILTER_H
#define HIGH_REFERENCE_FILTER_H

#include "project.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HIGH_REFERENCE_FILTER_ORDER 97 // To store up to x[n-96]

typedef struct {
	int32_t x[HIGH_REFERENCE_FILTER_ORDER]; // Input buffer for x[n] ... x[n-96]
	int32_t y[2]; // Output buffer for y[n-1] and y[n-2]
	int index;	  // Current index in the circular input buffer
} RCfilter_high_t;

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
int32_t high_reference_filter_update(RCfilter_high_t *filt, int32_t input);

#ifdef __cplusplus
}
#endif

#endif /* HIGH_REFERENCE_FILTER_H */
