#ifndef WORD_SEQUENCE_H
#define WORD_SEQUENCE_H

#include "project.h"

/**
 * Word sequence generation from symbolic sequence
 * Creates 3-symbol words from symbolic dynamics output
 * @param sy: Input symbolic sequence array (0-9)
 * @param wv: Output word sequence array
 * @param len: Signal length
 */
void word_sequence(const uint8_t *sy, WFDB_Time *wv, WFDB_Time len);

#endif /* WORD_SEQUENCE_H */
