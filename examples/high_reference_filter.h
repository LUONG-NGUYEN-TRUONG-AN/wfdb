#ifndef HIGH_REFERENCE_FILTER_H
#define HIGH_REFERENCE_FILTER_H

#include "project.h"

/**
 * High reference filter implementation: y[n] = (y[n-1] * 2) - y[n-2] + x[n] - x[n-32] - x[n-64] - x[n-96] with 47-sample output delay
 * @param input: Input signal array
 * @param output: Output signal array  
 * @param length: Signal length
 */
void high_reference_filter(const int32_t *input, int32_t *output, int32_t length);

#endif /* HIGH_REFERENCE_FILTER_H */
