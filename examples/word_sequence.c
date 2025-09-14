#include "word_sequence.h"

void word_sequence(const uint8_t *sy, WFDB_Time *wv, WFDB_Time len)
{
    if (!sy || !wv || len <= 0)
    {
        fprintf(stderr, "word_sequence: Invalid input parameters\n");
        return;
    }

    for (int32_t i = 0; i < len; ++i)
    {
        uint32_t i2 = (i - 2 < 0) ? 0 : (i - 2);
        uint32_t i1 = (i - 1 < 0) ? 0 : (i - 1);

        // Ensure symbols are within valid range [0..9]
        uint32_t s2 = (sy[i2] >= 0 && sy[i2] <= 9) ? sy[i2] : 0;
        uint32_t s1 = (sy[i1] >= 0 && sy[i1] <= 9) ? sy[i1] : 0;
        uint32_t s0 = (sy[i] >= 0 && sy[i] <= 9) ? sy[i] : 0;

        // Create word sequence
        WFDB_Time word_value = (s2 << 8) | (s1 << 4) | s0;

        // Ensure the word value doesn't exceed the buffer limits in shannon_entropy_algorithm
        // max_buffer is 2458, so this should be safe, but let's double-check
        if (word_value >= 2458)
        {
            fprintf(stderr, "word_sequence: Warning - word value %lld exceeds buffer limit, clamping to 2457\n", word_value);
            word_value = 2457;
        }

        wv[i] = word_value;
    }
}
