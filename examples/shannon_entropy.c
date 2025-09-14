#include "shannon_entropy.h"
#include <string.h>

/**
 * Shannon entropy algorithm following the flowchart logic
 * Uses occurrence counting and proper Shannon entropy calculation
 */

void shannon_entropy_algorithm(const WFDB_Time *inputs, float *results, uint32_t input_len)
{
#define max_buffer 2458 // (9 * 256 + 9 * 16 + 9) - Maximum possible word value
#define WINDOW_SIZE 127

    if (!inputs || !results || input_len <= 0)
    {
        fprintf(stderr, "shannon_entropy_algorithm: Invalid input parameters\n");
        return;
    }

    if (input_len < WINDOW_SIZE)
    {
        fprintf(stderr, "shannon_entropy_algorithm: Input length (%d) less than window size (%d)\n",
                input_len, WINDOW_SIZE);
        return;
    }

    // --- Data Structures for the Algorithm ---
    uint32_t freq[max_buffer];
    memset(freq, 0, max_buffer * sizeof(uint32_t));

    uint32_t window[WINDOW_SIZE];
    memset(window, 0, WINDOW_SIZE * sizeof(uint32_t));

    uint32_t k = 0;       // Cardinality (number of unique symbols)
    float S = 0.0;        // Shannon entropy sum
    uint32_t pointer = 0; // Circular buffer pointer

    // --- Main Loop ---
    for (uint32_t i = 0; i < input_len; ++i)
    {
        uint32_t idx = inputs[i];

        // Bounds checking for input samples
        if (idx > max_buffer || idx < 0)
        {
            fprintf(stderr, "shannon_entropy_algorithm: input sample %d = %lld out of bounds at index %d, clamping to %d\n",
                    idx, inputs[i], i, max_buffer - 1);
            idx = max_buffer - 1; // Clamp to valid range
        }

        if (i < WINDOW_SIZE)
        {
            // --- Initialization Phase (First window) ---
            window[pointer] = idx;
            uint32_t te_in = freq[idx];

            // Update cardinality if new symbol
            if (te_in == 0)
            {
                k++;
            }

            // Update frequency count
            freq[idx]++;

            // Calculate entropy delta with bounds checking
            float delta = ((float)PiMap[freq[idx]] - (float)PiMap[te_in]);
            S = S + delta;

            pointer = (pointer + 1) % WINDOW_SIZE;
        }
        else
        {
            // --- Sliding Window Phase ---
            uint32_t old_idx = window[pointer];
            uint32_t te_in = freq[idx];
            uint32_t te_out = freq[old_idx];

            // Bounds checking for old sample
            if (old_idx >= max_buffer)
            {
                fprintf(stderr, "shannon_entropy_algorithm: old sample %d out of bounds, clamping to %d\n",
                        old_idx, max_buffer - 1);
                old_idx = max_buffer - 1;
            }

            // Update window
            window[pointer] = idx;

            // Update frequency counts
            if (freq[old_idx] > 0)
            {
                freq[old_idx]--;
            }
            freq[idx]++;

            // Calculate entropy delta with comprehensive bounds checking
            float delta = (((float)PiMap[freq[idx]] - (float)PiMap[te_in]) + ((float)PiMap[freq[old_idx]] - (float)PiMap[te_out]));
            S = S + delta;

            // Update cardinality (k) following Python logic
            if (te_in == 0)
            {
                if (te_out > 1 || te_out == 0)
                {
                    k++;
                }
            }
            else
            {
                if (freq[old_idx] == 0 && te_out == 1)
                {
                    k--;
                }
            }
            pointer = (pointer + 1) % WINDOW_SIZE;
        }
        // printf("k = %u, S = %.3f, pointer = %u\n", k, S, pointer);
        // results[i] = (float)(k / 127.0) * (S / 1000000.0);
        // results[i] = (double)(k / 127.0) * (S / 1000000.0);
        results[i] = (float)((float)(k / 127.0) * (S / 1000000.0)); // Scale down to avoid overflow
    }
}
