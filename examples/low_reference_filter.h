#ifndef LOW_REFERENCE_FILTER_H
#define LOW_REFERENCE_FILTER_H

#include "project.h"

/**
 * Low reference filter implementation: y[n] = y[n-1] + x[n] - x[n-16] with 8-sample output delay
 * @param input: Input signal array
 * @param output: Output signal array
 * @param length: Signal length
 */
void low_reference_filter(const int32_t *input, int32_t *output, int32_t length);

#endif /* LOW_REFERENCE_FILTER_H */
