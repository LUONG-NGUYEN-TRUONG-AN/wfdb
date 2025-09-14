#ifndef SHANNON_ENTROPY_H
#define SHANNON_ENTROPY_H

#include "project.h"

/**
 * Shannon entropy algorithm following the flowchart logic
 * Uses occurrence counting and proper Shannon entropy calculation
 * @param inputs: Input word sequence array
 * @param results: Output entropy values array
 * @param input_len: Length of input array
 */
void shannon_entropy_algorithm(const WFDB_Time *inputs, float *results, uint32_t input_len);

#endif /* SHANNON_ENTROPY_H */
