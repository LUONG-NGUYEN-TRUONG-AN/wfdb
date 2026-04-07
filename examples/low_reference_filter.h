#ifndef LOW_REFERENCE_FILTER_H
#define LOW_REFERENCE_FILTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOW_REFERENCE_FILTER_ORDER  16u          /* must be a power of 2 */
#define LOW_REFERENCE_FILTER_MASK   (LOW_REFERENCE_FILTER_ORDER - 1u)  /* 0x0F */
#define LOW_REFERENCE_FILTER_SHIFT  4u           /* log2(16) — replaces /16 */

typedef struct {
    uint16_t x[LOW_REFERENCE_FILTER_ORDER];
    int32_t  acc;
    uint8_t  index;
} RCfilter_low_t;

/**
 * @brief Initialise the low-pass reference filter (zero state).
 * @param filt  Pointer to filter state.
 */
void low_reference_filter_init(RCfilter_low_t *filt);

/**
 * @brief Process one input sample through the low-pass reference filter.
 *
 * @param filt   Pointer to filter state.
 * @param input  New input sample x[n].
 * @return       Filtered, gain-normalised output y[n] = acc[n] / 16.
 */
uint16_t low_reference_filter_update(RCfilter_low_t *filt, uint16_t input);

#ifdef __cplusplus
}
#endif

#endif /* LOW_REFERENCE_FILTER_H */
