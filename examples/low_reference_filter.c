#include "low_reference_filter.h"

/**
 * Low reference filter implementation: y[n] = y[n-1] + x[n] - x[n-16] with 8-sample output delay
 * @param input: Input signal array
 * @param output: Output signal array
 * @param length: Signal length
 */
void low_reference_filter(const int32_t *input, int32_t *output, int32_t length) {
    if (!input || !output || length <= 0) {
        fprintf(stderr, "low_reference_filter: Invalid input parameters\n");
        return;
    }

    #define DELAY_SAMPLES 16
    #define LOW_OUTPUT_DELAY 8
    
    // Allocate temporary buffer for intermediate results
    int32_t *temp_output;
    SUALLOC(temp_output, length, sizeof(int32_t));
    if (!temp_output) {
        fprintf(stderr, "low_reference_filter: temp_output allocation failed\n");
        return;
    }
    
    // Compute filter without delay first
    for (uint32_t n = 0; n < length; n++) {
        int32_t prev_output = (n > 0) ? temp_output[n-1] : 0;  // y[n-1], assume y[-1] = 0
        int32_t delayed_input = (n >= DELAY_SAMPLES) ? input[n - DELAY_SAMPLES] : 0;  // x[n-16], zero-pad

        // Implement the exact difference equation: y[n] = y[n-1] + x[n] - x[n-16]
        temp_output[n] = prev_output + input[n] - delayed_input;
    }
    
    // Apply scaling
    for (uint32_t n = 0; n < length; n++) {
        temp_output[n] = temp_output[n] >> 4;  // output[n] / SCALE_16 (16 = 2^4)
    }
    
    // Apply 8-sample output delay: shift the filtered result by 8 samples
    for (uint32_t n = 0; n < length; n++) {
        if (n < LOW_OUTPUT_DELAY) {
            // For the first 8 samples, use filtered values from the beginning
            output[n] = temp_output[0];
        } else {
            // For samples >= 8, use filtered values from 8 samples earlier
            output[n] = temp_output[n - LOW_OUTPUT_DELAY];
        }
    }
    
    SFREE(temp_output);
}
