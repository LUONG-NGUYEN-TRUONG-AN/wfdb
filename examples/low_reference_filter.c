#include "low_reference_filter.h"
#include <string.h>

void low_reference_filter_init(RCfilter_low_t *filt)
{
    if (!filt) return;
    memset(filt->x, 0, sizeof(filt->x));
    filt->acc   = 0;
    filt->index = 0;
}

/**
 * Low-pass reference filter: 16-tap moving average (CIC order-1, N=16)
 *
 * Difference equation:
 *   acc[n] = acc[n-1] + x[n] - x[n-16]
 *   y[n]   = acc[n] >> 4          (divide by 16 = unity pass-band gain)
 *
 * Key fixes vs. previous version:
 *  1. Accumulator is int32_t  — prevents uint16_t overflow/underflow wrapping.
 *  2. acc stores the raw (×16) sum; only the RETURNED value is scaled.
 *     The feedback path no longer mixes scaled and unscaled values.
 *  3. Circular index uses bitmask (& 0x0F) instead of modulo — faster on
 *     Xtensa LX7 / Cortex-M which lack a single-cycle divide.
 *  4. Subtraction is done in signed 32-bit — no silent underflow.
 *
 * Group delay: 7.5 samples (N/2 - 0.5).
 *
 * @param filt   Pointer to filter state.
 * @param input  New input sample x[n].
 * @return       Gain-normalised output y[n].
 */
uint16_t low_reference_filter_update(RCfilter_low_t *filt, uint16_t input)
{
    if (!filt) return 0;

    uint16_t oldest = filt->x[filt->index];

    filt->x[filt->index] = input;

    filt->index = (uint8_t)((filt->index + 1u) & LOW_REFERENCE_FILTER_MASK);

    filt->acc += (int32_t)input - (int32_t)oldest;

    return (uint16_t)(filt->acc >> LOW_REFERENCE_FILTER_SHIFT);
}
