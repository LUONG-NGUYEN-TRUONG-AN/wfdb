#ifndef LOW_REFERENCE_FILTER_H
#define LOW_REFERENCE_FILTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOW_REFERENCE_FILTER_ORDER 16

typedef struct {
	uint16_t x[LOW_REFERENCE_FILTER_ORDER]; // Input buffer (acts as a circular
										   // buffer)
	uint16_t output;						   // Last output value y[n-1]
	uint8_t index; // Current index in the circular buffer
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
 */
uint16_t low_reference_filter_update(RCfilter_low_t *filt, const uint16_t input);

#ifdef __cplusplus
}
#endif

#endif /* LOW_REFERENCE_FILTER_H */
