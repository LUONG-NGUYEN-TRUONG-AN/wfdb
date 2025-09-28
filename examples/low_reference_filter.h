#ifndef LOW_REFERENCE_FILTER_H
#define LOW_REFERENCE_FILTER_H

#include "project.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOW_REFERENCE_FILTER_ORDER 16

typedef struct {
	int32_t x[LOW_REFERENCE_FILTER_ORDER]; // Input buffer (acts as a circular
										   // buffer)
	int32_t output;						   // Last output value y[n-1]
	int32_t index; // Current index in the circular buffer
} RCfilter_low_t;

/**
 * @brief Initialize the low reference filter
 *
 * @param filt: Filter parameter
 */
void low_reference_filter_init(RCfilter_low_t *filt);

/**
 * Low reference filter implementation: y[n] = y[n-1] + x[n] - x[n-16] with
 * 8-sample output delay
 * @param input: Input signal array
 * @param output: Output signal array
 * @param length: Signal length
 */
int32_t low_reference_filter_update(RCfilter_low_t *filt, const int32_t input);

#ifdef __cplusplus
}
#endif

#endif /* LOW_REFERENCE_FILTER_H */
