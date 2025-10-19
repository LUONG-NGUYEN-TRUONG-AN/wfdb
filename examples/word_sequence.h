#ifndef WORD_SEQUENCE_H
#define WORD_SEQUENCE_H

#include "project.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // State structure to hold the history of the last two symbols
    typedef struct
    {
        uint8_t s1; // Previous symbol, sy[n-1]
        uint8_t s2; // Symbol before previous, sy[n-2]
    } word_sequence_state_t;

    void word_sequence_init(word_sequence_state_t *state);
    uint16_t word_sequence_update(word_sequence_state_t *state, uint8_t current_sy);

#ifdef __cplusplus
}
#endif

#endif /* WORD_SEQUENCE_H */
