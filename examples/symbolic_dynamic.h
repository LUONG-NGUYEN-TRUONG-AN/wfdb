#ifndef SYMBOLIC_DYNAMIC_H
#define SYMBOLIC_DYNAMIC_H

#include "project.h"

/**
 * Symbolic dynamics implementation that converts signal differences into symbolic representation
 * @param x: Original RR interval signal
 * @param xl: Low-reference filtered signal
 * @param xh: High-reference filtered signal (used for thresholds)
 * @param sy: Output symbolic sequence array (0-9)
 * @param len: Signal length
 */
void symbolic_dynamic(const WFDB_Time *x, const int32_t *xl, const int32_t *xh, uint8_t *sy, WFDB_Time len);

// Helper function to ensure the "gen" directory exists
void ensure_gen_directory(void);

#endif /* SYMBOLIC_DYNAMIC_H */
