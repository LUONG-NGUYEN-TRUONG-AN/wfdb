#include "high_reference_filter.h"
#include <string.h>

/**
 * High reference filter implementation: y[n] = (y[n-1] * 2) - y[n-2] + x[n] - x[n-32] - x[n-64] - x[n-96] with 47-sample output delay
 * @param input: Input signal array
 * @param output: Output signal array  
 * @param length: Signal length
 */
void high_reference_filter(const int32_t *input, int32_t *output, int32_t length) {
    if (!input || !output || length <= 0) {
        fprintf(stderr, "high_reference_filter: Invalid input parameters\n");
        return;
    }

    #define DELAY_32 32
    #define DELAY_64 64
    #define DELAY_96 96
    #define HIGH_OUTPUT_DELAY 47

    // Allocate temporary buffer for intermediate results
    int32_t *temp_output;
    SUALLOC(temp_output, length, sizeof(int32_t));
    if (!temp_output) {
        fprintf(stderr, "high_reference_filter: temp_output allocation failed\n");
        return;
    }

    // Protect against in-place use (input == output)
    const int32_t *src = input;
    int32_t *tmp = NULL;
    if (input == temp_output) {
        SUALLOC(tmp, length, sizeof(int32_t));
        if (!tmp) {
            fprintf(stderr, "high_reference_filter: temp allocation failed for in-place call\n");
            SFREE(temp_output);
            return;
        }
        // Copy input to tmp once
        memcpy(tmp, input, (size_t)length * sizeof(int32_t));
        src = tmp;
    } else {
        src = input;
    }

    int32_t n = 0;

    // Transient region with zero-padding (n < 96)
    for (; n < length && n < DELAY_96; ++n) {
        int32_t prev_y1 = (n > 0) ? temp_output[n-1] : 0;
        int32_t prev_y2 = (n > 1) ? temp_output[n-2] : 0;
        int32_t x_n32 = (n >= DELAY_32) ? src[n - DELAY_32] : 0;
        int32_t x_n64 = (n >= DELAY_64) ? src[n - DELAY_64] : 0;
        int32_t x_n96 = 0; // n < 96 here
        temp_output[n] = (prev_y1 * 2) - prev_y2 + src[n] - x_n32 - x_n64 + x_n96;
    }

    // Steady-state region: no boundary checks needed (n >= 96)
    for (; n < length; ++n) {
        int32_t prev_y1 = temp_output[n-1];
        int32_t prev_y2 = temp_output[n-2];
        temp_output[n] = (prev_y1 * 2) - prev_y2
                    + src[n] - src[n - DELAY_32] - src[n - DELAY_64] + src[n - DELAY_96];
    }
    
    // Normalization using bit shift (divide by 2048)
    for (WFDB_Time i = 0; i < length; ++i) {
        temp_output[i] = temp_output[i] >> 11;
    }

    // Apply 47-sample output delay: shift the filtered result by 47 samples
    for (uint32_t n = 0; n < length; n++) {
        if (n < HIGH_OUTPUT_DELAY) {
            // For the first 47 samples, use filtered values from the beginning
            output[n] = temp_output[0];
        } else {
            // For samples >= 47, use filtered values from 47 samples earlier
            output[n] = temp_output[n - HIGH_OUTPUT_DELAY];
        }
    }

    if (tmp) {
        SFREE(tmp);
    }
    SFREE(temp_output);
}
