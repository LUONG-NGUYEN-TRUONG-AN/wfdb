#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

#include "project.h"

/**
 * Median filter implementation with 8-sample output delay
 * @param x: Input signal array
 * @param output: Output signal array (median filtered)
 * @param n: Signal length
 */
void median_filter(const WFDB_Time *x, int32_t *output, int32_t n);

#endif /* MEDIAN_FILTER_H */
