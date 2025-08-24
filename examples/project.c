#include "project.h"
#include "test.h"

WFDB_Time PiMap[127] = {  7874,  13495, 18265, 22483, 26290, 29770, 32977, 35952,
                    38723, 41313, 43740, 46019, 48162, 50181, 52083, 53877,
                    55569, 57165, 58671, 60092, 61431, 62693, 63880, 64997,
                    66047, 67031, 67953, 68815, 69618, 70366, 71059, 71700,
                    72290, 72830, 73323, 73770, 74171, 74529, 74843, 75116,
                    75348, 75541, 75695, 75811, 75890, 75933, 75941, 75914,
                    75854, 75760, 75633, 75475, 75285, 75065, 74815, 74535,
                    74226, 73889, 73523, 73130, 72710, 72263, 71790, 71292,
                    70767, 70218, 69645, 69046, 68425, 67779, 67110, 66419,
                    65704, 64968, 64209, 63429, 62628, 61805, 60962, 60098,
                    59213, 58309, 57385, 56441, 55478, 54495, 53494, 52474,
                    51436, 50379, 49305, 48212, 47102, 45974, 44829, 43667,
                    42488, 41292, 40080, 38851, 37606, 36345, 35068, 33775,
                    32467, 31143, 29803, 28449, 27079, 25695, 24296, 22882,
                    21453, 20011, 18553, 17082, 15597, 14098, 12585, 11059,
                    9519,  7965,  6398,  4818,  3225,  1619,   0
                    };

WFDB_Time icmp(WFDB_Time *x, WFDB_Time *y)
{
    return (*y - *x);
}

static int compare(const void *a, const void *b) {
    int diff = *(const int*)a - *(const int*)b;
    return (diff > 0) - (diff < 0);
}

void median_filter(const WFDB_Time *input, WFDB_Time *output, const WFDB_Time length, const WFDB_Time flen) {
    // Input validation
    assert(input != NULL && output != NULL);
    assert(length > 0 && flen > 0 && (flen & 1));
    
    WFDB_Time median_idx = flen >> 1;  // flen / 2
    WFDB_Time *window;
    SUALLOC(window, flen, sizeof(WFDB_Time));
    if (!window) {
        fprintf(stderr, "median_filter: window allocation failed\n");
        return;
    }

    for (uint32_t i = 0; i < length; i++) {
        // Fill window with available samples (nearest-neighbor padding)
        for (uint32_t j = 0; j < flen; j++) {
            WFDB_Time idx = i - median_idx + j;
            if (idx < 0) idx = 0;
            if (idx >= length) idx = length - 1;
            window[j] = (WFDB_Time)input[idx];
        }
        
        // Sort window using proper WFDB_Time comparison
        qsort(window, flen, sizeof(WFDB_Time), compare);
        
        // Store median
        output[i] = window[median_idx];
    }
    
    SFREE(window);
}

/**
 * Low reference filter implementation: y[n] = y[n-1] + x[n] - x[n-16] with 8-sample output delay
 * @param input: Input signal array
 * @param output: Output signal array
 * @param length: Signal length
 */
void low_reference_filter(const WFDB_Time *input, WFDB_Time *output, WFDB_Time length) {
    if (!input || !output || length <= 0) {
        fprintf(stderr, "low_reference_filter: Invalid input parameters\n");
        return;
    }

    #define DELAY_SAMPLES 16
    #define OUTPUT_DELAY 8
    
    // Allocate temporary buffer for intermediate results
    WFDB_Time *temp_output;
    SUALLOC(temp_output, length, sizeof(WFDB_Time));
    if (!temp_output) {
        fprintf(stderr, "low_reference_filter: temp_output allocation failed\n");
        return;
    }
    
    // Compute filter without delay first
    for (uint32_t n = 0; n < length; n++) {
        WFDB_Time prev_output = (n > 0) ? temp_output[n-1] : 0;  // y[n-1], assume y[-1] = 0
        WFDB_Time delayed_input = (n >= DELAY_SAMPLES) ? input[n - DELAY_SAMPLES] : 0;  // x[n-16], zero-pad
            
        // Implement the exact difference equation: y[n] = y[n-1] + x[n] - x[n-16]
        temp_output[n] = prev_output + input[n] - delayed_input;
    }
    
    // Apply scaling
    for (uint32_t n = 0; n < length; n++) {
        temp_output[n] = temp_output[n] >> 4;  // output[n] / SCALE_16 (16 = 2^4)
    }
    
    // Apply 8-sample output delay: shift the filtered result by 8 samples
    for (uint32_t n = 0; n < length; n++) {
        if (n < OUTPUT_DELAY) {
            // For the first 8 samples, use filtered values from the beginning
            output[n] = temp_output[0];
        } else {
            // For samples >= 8, use filtered values from 8 samples earlier
            output[n] = temp_output[n - OUTPUT_DELAY];
        }
    }
    
    SFREE(temp_output);
}

/**
 * High reference filter implementation: y[n] = (y[n-1] * 2) - y[n-2] + x[n] - x[n-32] - x[n-64] - x[n-96] with 47-sample output delay
 * @param input: Input signal array
 * @param output: Output signal array  
 * @param length: Signal length
 */
void high_reference_filter(const WFDB_Time *input, WFDB_Time *output, WFDB_Time length) {
    if (!input || !output || length <= 0) {
        fprintf(stderr, "high_reference_filter: Invalid input parameters\n");
        return;
    }

    #define DELAY_32 32
    #define DELAY_64 64
    #define DELAY_96 96
    #define HIGH_OUTPUT_DELAY 47

    // Allocate temporary buffer for intermediate results
    WFDB_Time *temp_output;
    SUALLOC(temp_output, length, sizeof(WFDB_Time));
    if (!temp_output) {
        fprintf(stderr, "high_reference_filter: temp_output allocation failed\n");
        return;
    }

    // Protect against in-place use (input == output)
    const WFDB_Time *src = input;
    WFDB_Time *tmp = NULL;
    if (input == temp_output) {
        SUALLOC(tmp, length, sizeof(WFDB_Time));
        if (!tmp) {
            fprintf(stderr, "high_reference_filter: temp allocation failed for in-place call\n");
            SFREE(temp_output);
            return;
        }
        // Copy input to tmp once
        memcpy(tmp, input, (size_t)length * sizeof(WFDB_Time));
        src = tmp;
    } else {
        src = input;
    }

    WFDB_Time n = 0;

    // Transient region with zero-padding (n < 96)
    for (; n < length && n < DELAY_96; ++n) {
        WFDB_Time prev_y1 = (n > 0) ? temp_output[n-1] : 0;
        WFDB_Time prev_y2 = (n > 1) ? temp_output[n-2] : 0;
        WFDB_Time x_n32 = (n >= DELAY_32) ? src[n - DELAY_32] : 0;
        WFDB_Time x_n64 = (n >= DELAY_64) ? src[n - DELAY_64] : 0;
        WFDB_Time x_n96 = 0; // n < 96 here
        temp_output[n] = (prev_y1 * 2) - prev_y2 + src[n] - x_n32 - x_n64 + x_n96;
    }

    // Steady-state region: no boundary checks needed (n >= 96)
    for (; n < length; ++n) {
        WFDB_Time prev_y1 = temp_output[n-1];
        WFDB_Time prev_y2 = temp_output[n-2];
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

void symbolic_dynamic(const WFDB_Time *x, const WFDB_Time *xl, const WFDB_Time *xh, WFDB_Time *sy, WFDB_Time len) {
    WFDB_Time *delta_x = NULL;
    SUALLOC(delta_x, len, sizeof(WFDB_Time));
    #define X_DELAY 63   // Delay for x array
    #define XL_DELAY 47  // Delay for xl array
    for (WFDB_Time i = 0; i < len; i++) {
        WFDB_Time x_delayed, xl_delayed;
        
        // Get delayed x value with bounds checking
        if (i >= X_DELAY) {
            x_delayed = (WFDB_Time)x[i - X_DELAY];
        } else {
            x_delayed = (WFDB_Time)x[0];  // Use first value for early samples
        }
        
        // Get delayed xl value with bounds checking  
        if (i >= XL_DELAY) {
            xl_delayed = xl[i - XL_DELAY];
        } else {
            xl_delayed = xl[0];  // Use first value for early samples
        }
        
        // Calculate delta with proper delay indexing
        delta_x[i] = x_delayed - xl_delayed;
    }
    for (uint32_t i = 0; i < len; i++) {
        WFDB_Time thres1 = xh[i] >> 4;  // xh[i] / 16 (16 = 2^4)
        WFDB_Time thres2 = xh[i] >> 3;  // xh[i] / 8 (8 = 2^3)
        WFDB_Time thres3 = thres1 + thres2;
        WFDB_Time thres4 = xh[i] >> 2;  // xh[i] / 4 (4 = 2^2)
        WFDB_Time thres5 = thres4 + thres1;

        if (delta_x[i] < -thres4) {
            sy[i] = 0;
        } else if (delta_x[i] < -thres3) {
            sy[i] = 1;
        } else if (delta_x[i] < -thres2) {
            sy[i] = 2;
        } else if (delta_x[i] < -thres1) {
            sy[i] = 3;
        } else if (delta_x[i] < thres1) {
            sy[i] = 4;
        } else if (delta_x[i] < thres2) {
            sy[i] = 5;
        } else if (delta_x[i] < thres3) {
            sy[i] = 6;
        } else if (delta_x[i] < thres4) {
            sy[i] = 7;
        } else if (delta_x[i] < thres5) {
            sy[i] = 8;
        } else {
            sy[i] = 9;
        }
    }
    SFREE(delta_x);
}

void word_sequence(const WFDB_Time *sy, WFDB_Time *wv, WFDB_Time len) {
    if (!sy || !wv || len <= 0) {
        fprintf(stderr, "word_sequence: Invalid input parameters\n");
        return;
    }
    for (uint32_t i = 0; i < len; ++i) {
        uint32_t i2 = (i - 2 < 0) ? 0 : (i - 2);
        uint32_t i1 = (i - 1 < 0) ? 0 : (i - 1);
        uint32_t s2 = sy[i2];
        uint32_t s1 = sy[i1];
        uint32_t s0 = sy[i];
        // sy expected in [0..9]; if broader inputs occur, consider clamping to [0..15].
        wv[i] = (s2 << 8) | (s1 << 4) | s0;
    }
}

/**
 * Shannon entropy algorithm following the flowchart logic
 * Uses occurrence counting and proper Shannon entropy calculation
 */
void shannon_entropy_algorithm(const WFDB_Time *inputs, float *results, uint32_t input_len) {
    #define max_buffer 2458 // (9 * 256 + 9 * 16 + 9) - Maximum possible word value
    #define WINDOW_SIZE 127
    #define PRINT_SHANNON_DEBUG 0  // Debug output control
    
    if (!inputs || !results || input_len <= 0) {
        fprintf(stderr, "shannon_entropy_algorithm: Invalid input parameters\n");
        return;
    }
    
    if (input_len < WINDOW_SIZE) {
        fprintf(stderr, "shannon_entropy_algorithm: Input length (%d) less than window size (%d)\n", 
                input_len, WINDOW_SIZE);
        return;
    }
    
    // --- Data Structures for the Algorithm ---
    WFDB_Time *freq;
    SUALLOC(freq, max_buffer, sizeof(WFDB_Time));
    if (!freq) {
        fprintf(stderr, "shannon_entropy_algorithm: freq allocation failed\n");
        return;
    }
    memset(freq, 0, max_buffer * sizeof(WFDB_Time));
    
    WFDB_Time *window;
    SUALLOC(window, WINDOW_SIZE, sizeof(WFDB_Time));
    if (!window) {
        fprintf(stderr, "shannon_entropy_algorithm: window allocation failed\n");
        SFREE(freq);
        return;
    }
    memset(window, 0, WINDOW_SIZE * sizeof(WFDB_Time));
    
    WFDB_Time k = 0;      // Cardinality (number of unique symbols)
    float S = 0.0;      // Shannon entropy sum
    WFDB_Time pointer = 0; // Circular buffer pointer

    // --- Main Loop ---
    for (uint32_t i = 0; i < input_len; ++i) {
        uint32_t idx = inputs[i];
        
        // Bounds checking for input samples
        if (idx >= max_buffer) {
            fprintf(stderr, "shannon_entropy_algorithm: input sample %d out of bounds at index %d, clamping to %d\n", 
                   idx, i, max_buffer - 1);
            idx = max_buffer - 1; // Clamp to valid range
        }
        
        if (i < WINDOW_SIZE) {
            // --- Initialization Phase (First window) ---
            window[pointer] = idx;
            WFDB_Time te_in = freq[idx];
            
            // Update cardinality if new symbol
            if (te_in == 0) {
                k++;
            }
            
            // Update frequency count
            freq[idx]++;
            
            // Calculate entropy delta with bounds checking
            float delta = ((float)PiMap[freq[idx]] - (float)PiMap[te_in]);
            S = S + delta;
            
            pointer = (pointer + 1) % WINDOW_SIZE;
        } else {
            // --- Sliding Window Phase ---
            WFDB_Time old_idx = window[pointer];
            WFDB_Time te_in = freq[idx];
            WFDB_Time te_out = freq[old_idx];
            
            // Bounds checking for old sample
            if (old_idx >= max_buffer) {
                fprintf(stderr, "shannon_entropy_algorithm: old sample %lld out of bounds, clamping to %d\n", 
                       old_idx, max_buffer - 1);
                old_idx = max_buffer - 1;
            }
            
            // Update window
            window[pointer] = idx;
            
            // Update frequency counts
            if (freq[old_idx] > 0) {
                freq[old_idx]--;
            }
            freq[idx]++;
            
            // Calculate entropy delta with comprehensive bounds checking            
            float delta = (((float)PiMap[freq[idx]] - (float)PiMap[te_in]) + ((float)PiMap[freq[old_idx]] - (float)PiMap[te_out]));
            S = S + delta;
            
            // Update cardinality (k) following Python logic
            if (te_in == 0) {
                if (te_out > 1 || te_out == 0) {
                    k++;
                }
            } else {
                if (freq[old_idx] == 0 && te_out == 1) {
                    k--;
                }
            }
            pointer = (pointer + 1) % WINDOW_SIZE;
        }
        // printf("k = %u, S = %.3f, pointer = %u\n", k, S, pointer);
        // results[i] = (float)(k / 127.0) * (S / 1000000.0);
        // results[i] = (double)(k / 127.0) * (S / 1000000.0);
        results[i] = (float)((float)(k * S) / 127000000.0); // Scale down to avoid overflow
    }
    SFREE(freq);
    SFREE(window);
}

/**
 * Build histogram (frequency count) for entropy_results.
 * - Computes min/max uint32_ternally.
 * - Allocates counts[num_bins] and edges[num_bins+1] via SALLOC; caller must SFREE both.
 * Returns 0 on success, -1 on error.
 */
static WFDB_Time entropy_histogram(const WFDB_Time *arr, WFDB_Time len, WFDB_Time num_bins,
                             WFDB_Time **counts_out, WFDB_Time **edges_out,
                             WFDB_Time *min_out, WFDB_Time *max_out) {
    if (!arr || len <= 0 || num_bins <= 0 || !counts_out || !edges_out || !min_out || !max_out) {
        return -1;
    }

    // Compute min/max
    WFDB_Time mn = arr[0], mx = arr[0];
    for (WFDB_Time i = 1; i < len; ++i) {
        if (arr[i] < mn) mn = arr[i];
        if (arr[i] > mx) mx = arr[i];
    }
    // Avoid zero-width range
    if (mx == mn) {
        mx = mn + 1;
    }

    WFDB_Time *edges; SALLOC(edges, (size_t)(num_bins + 1), sizeof(WFDB_Time));
    WFDB_Time *counts;   SALLOC(counts, (size_t)num_bins, sizeof(WFDB_Time));
    if (!edges || !counts) {
        if (edges) SFREE(edges);
        if (counts) SFREE(counts);
        return -1;
    }
    memset(counts, 0, num_bins * sizeof(WFDB_Time));

    // Protect against division by zero
    const WFDB_Time width = (num_bins > 0 && mx > mn) ? (mx - mn) / num_bins : 1;
    for (WFDB_Time i = 0; i <= num_bins; ++i) {
        edges[i] = mn + width * i;
    }

    // Accumulate counts
    for (WFDB_Time i = 0; i < len; ++i) {
        WFDB_Time idx = (width > 0) ? (arr[i] - mn) / width : 0;
        if (idx < 0) idx = 0;
        if (idx >= num_bins) idx = num_bins - 1; // include right edge
        counts[idx]++;
    }

    *counts_out = counts;
    *edges_out = edges;
    *min_out = mn;
    *max_out = mx;
    return 0;
}

/**
 * Initialize performance metrics structure
 * @param metrics: Pointer to performance metrics structure
 */
void performance_metrics_init(performance_metrics_t *metrics) {
    if (!metrics) {
        fprintf(stderr, "performance_metrics_init: Invalid input parameter\n");
        return;
    }
    
    metrics->TP = 0;
    metrics->FP = 0;
    metrics->FN = 0;
    metrics->TN = 0;
    metrics->Se = 0;
    metrics->Sp = 0;
    metrics->PPV = 0;
    metrics->ACC = 0;
}

/**
 * Update confusion matrix with new predictions
 * @param metrics: Pointer to performance metrics structure
 * @param actual: Array of actual labels (0 or 1)
 * @param predicted: Array of predicted labels (0 or 1)
 * @param length: Length of arrays
 */
void performance_metrics_update(performance_metrics_t *metrics, const uint8_t *actual, const WFDB_Time *predicted, size_t length) {
    if (!metrics || !actual || !predicted || length == 0) {
        fprintf(stderr, "performance_metrics_update: Invalid input parameters\n");
        return;
    }
    
    for (size_t i = 0; i < length; i++) {
        if ((actual[i] == 1) && (predicted[i] == 1)) {
            metrics->TP++;
        } else if ((actual[i] == 0) && (predicted[i] == 1)) {
            metrics->FP++;
        } else if ((actual[i] == 1) && (predicted[i] == 0)) {
            metrics->FN++;
        } else if ((actual[i] == 0) && (predicted[i] == 0)) {
            metrics->TN++;
        }
    }
}

/**
 * Calculate performance metrics from confusion matrix
 * @param metrics: Pointer to performance metrics structure
 */
void performance_metrics_calculate(performance_metrics_t *metrics) {
    if (!metrics) {
        fprintf(stderr, "performance_metrics_calculate: Invalid input parameter\n");
        return;
    }
    
    // Calculate performance metrics with division by zero protection
    metrics->Se = (metrics->TP + metrics->FN > 0) ? (metrics->TP * 100) / (metrics->TP + metrics->FN) : 0;
    metrics->Sp = (metrics->TN + metrics->FP > 0) ? (metrics->TN * 100) / (metrics->TN + metrics->FP) : 0;
    metrics->PPV = (metrics->TP + metrics->FP > 0) ? (metrics->TP * 100) / (metrics->TP + metrics->FP) : 0;
    metrics->ACC = (metrics->TP + metrics->TN + metrics->FP + metrics->FN > 0) ? 
                   ((metrics->TP + metrics->TN) * 100) / (metrics->TP + metrics->TN + metrics->FP + metrics->FN) : 0;
}

/**
 * Print performance metrics to console
 * @param metrics: Pointer to performance metrics structure
 * @param record_name: Name of the record
 */
void performance_metrics_print(const performance_metrics_t *metrics, const char *record_name) {
    if (!metrics || !record_name) {
        fprintf(stderr, "performance_metrics_print: Invalid input parameters\n");
        return;
    }
    
    printf("=== Performance Metrics for Record: %s ===\n", record_name);
    printf("TP: %lld, FP: %lld, FN: %lld, TN: %lld\n", metrics->TP, metrics->FP, metrics->FN, metrics->TN);
    printf("Sensitivity (Se): %lld%%, Specificity (Sp): %lld%%, PPV: %lld%%, Accuracy: %lld%%\n",
           metrics->Se, metrics->Sp, metrics->PPV, metrics->ACC);
    printf("================================================\n");
}

/**
 * Write performance metrics to file
 * @param metrics: Pointer to performance metrics structure
 * @param record_name: Name of the record
 */
void performance_metrics_write_to_file(const performance_metrics_t *metrics, const char *record_name) {
    if (!metrics || !record_name) {
        fprintf(stderr, "performance_metrics_write_to_file: Invalid input parameters\n");
        return;
    }
    
    FILE *file = fopen("result.txt", "a"); // Append mode to accumulate results from multiple records
    if (file) {
        fprintf(file, "Record: %s\n", record_name);
        fprintf(file, "TP: %lld, FP: %lld, FN: %lld, TN: %lld\n", 
               metrics->TP, metrics->FP, metrics->FN, metrics->TN);
        fprintf(file, "Sensitivity (Se): %lld%%, Specificity (Sp): %lld%%, PPV: %lld%%, Accuracy: %lld%%\n",
               metrics->Se, metrics->Sp, metrics->PPV, metrics->ACC);
        fprintf(file, "-------------------------------------------\n");
        fflush(file);
        fclose(file);
    } else {
        fprintf(stderr, "Error: Could not open result.txt for writing\n");
    }
}

/**
 * Export actual vs predicted labels to text file for comparison
 * @param actual: Array of actual labels (0 or 1)
 * @param predicted: Array of predicted labels (0 or 1) 
 * @param length: Length of arrays
 * @param record_name: Name of the record
 */
void export_comparison_to_file(const uint8_t *actual, const WFDB_Time *predicted, size_t length, const char *record_name) {
    if (!actual || !predicted || length == 0 || !record_name) {
        fprintf(stderr, "export_comparison_to_file: Invalid input parameters\n");
        return;
    }
    
    char filename[256];
    snprintf(filename, sizeof(filename), "comparison_%s.txt", record_name);
    
    FILE *file = fopen(filename, "w");
    if (file) {
        // Write header with fixed-width columns
        fprintf(file, "# Record: %s\n", record_name);
        fprintf(file, "# %-10s %-10s %-10s %-10s\n", "Index", "Actual", "Predicted", "Match");
        fprintf(file, "# ---------- ---------- ---------- ----------\n");
        
        // Write data with fixed-width formatting
        for (size_t i = 0; i < length; i++) {
            uint8_t predicted_u8 = (uint8_t)predicted[i];
            uint8_t match = (actual[i] == predicted_u8) ? 1 : 0;
            fprintf(file, "  %-10zu %-10u %-10u %-10u\n", i, actual[i], predicted_u8, match);
        }
        
        fflush(file);
        fclose(file);
        printf("Comparison data exported to %s\n", filename);
    } else {
        fprintf(stderr, "Error: Could not open %s for writing\n", filename);
    }
}

#ifndef SKIP_MAIN
void main() {
    #if (DATABASE == MITDB)
        setwfdb("https://physionet.org/files/mitdb/1.0.0/");
        // Complete MITDB records array from RECORDS file
        const char *records[] = {
            "100", "101", "102", "103", "104", "105", "106", "107", "108", "109",
            "111", "112", "113", "114", "115", "116", "117", "118", "119", "121",
            "122", "123", "124", "200", "201", "202", "203", "205", "207", "208",
            "209", "210", "212", "213", "214", "215", "217", "219", "220", "221",
            "222", "223", "228", "230", "231", "232", "233", "234"
        };
        // const char *records[] = {"201", "202", "203", "210", "217", "219", "221", "222"};

    #elif (DATABASE == AFDB)
        setwfdb("https://physionet.org/files/afdb/1.0.0/");
        // const char *records[] = {"00735"};
        // const char *records[] = {"04043"};
        const char *records[] = {"00735", "03665", "04015", "04043", "04048", "04126", "04746", "04908", "04936", "05091", "05121", "05261", "06426", "06453", "06995", "07162", "07859", "07879", "07910", "08215", "08219", "08378", "08405", "08434", "08455"};
    #endif
    uint8_t record_size_u8 = sizeof(records) / sizeof(records[0]);
    // {'N', 'L', 'R', 'a', 'V', 'F', 'J', 'A', 'S', 'E', 'j', '/', 'Q'};
    const char **record = records;
    const uint8_t a0f_u8[ACMAX+1] = {  /* annotation filter array for uint32_terval start pouint32_ts */
                                1,	1,	1,	1,	1,		/* 0 - 4 */
                                1,	1,	1,	1,	1,		/* 5 - 9 */
                                1,	1,	1,	1,	0,		/* 10 - 14 */
                                0,	0,	0,	0,	0,		/* 15 - 19 */
                                0,	0,	0,	0,	0,		/* 20 - 24 */
                                0,	0,	0,	0,	0,		/* 25 - 29 */
                                0,	0,	0,	0,	0,		/* 30 - 34 */
                                0,	0,	0,	0,	0,		/* 35 - 39 */
                                0,	0,	0,	0,	0,		/* 40 - 44 */
                                0,	0,	0,	0,	0		/* 45 - 49 */
                        };
    const uint8_t a1f_u8[ACMAX+1] = {  /* annotation filter array for uint32_terval start pouint32_ts */
                                1,	1,	1,	1,	1,		/* 0 - 4 */
                                1,	1,	1,	1,	1,		/* 5 - 9 */
                                1,	1,	1,	1,	0,		/* 10 - 14 */
                                0,	0,	0,	0,	0,		/* 15 - 19 */
                                0,	0,	0,	0,	0,		/* 20 - 24 */
                                0,	0,	0,	0,	0,		/* 25 - 29 */
                                0,	0,	0,	0,	0,		/* 30 - 34 */
                                0,	0,	0,	0,	0,		/* 35 - 39 */
                                0,	0,	0,	0,	0,		/* 40 - 44 */
                                0,	0,	0,	0,	0		/* 45 - 49 */
                        };

    if (record != NULL) {
        #if (DATABASE == MITDB)
        WFDB_Anninfo a;
        #elif (DATABASE == AFDB)
        WFDB_Anninfo a[2];
        #endif
        
        WFDB_Annotation annot;
        WFDB_Time t0, t1;
        uint8_t *actual_u8 = NULL;
        
        WFDB_Time t0_u32 = 0,
                  t1_u32 = 0,
                  a0_u32 = 0, 
                  a1_u32 = 0;
        unsigned char start_afib = 0;
        performance_metrics_t metrics;
        performance_metrics_init(&metrics);
        size_t rr_intervals_capacity = 1000;  // Initial capacity
        struct rr_intervals *rr_intervals = NULL;
        SUALLOC(rr_intervals, 1, sizeof(struct rr_intervals));
        if (!rr_intervals) {
            fprintf(stderr, "Error: rr_intervals allocation failed\n");
            exit(1);
        }
        rr_intervals->length = 0;
        SUALLOC(rr_intervals->x, rr_intervals_capacity, sizeof(WFDB_Time));
        SUALLOC(rr_intervals->time, rr_intervals_capacity, sizeof(WFDB_Time));
        if (!rr_intervals->x || !rr_intervals->time) {
            fprintf(stderr, "Error: rr_intervals arrays allocation failed\n");
            if (rr_intervals->x) SFREE(rr_intervals->x);
            if (rr_intervals->time) SFREE(rr_intervals->time);
            SFREE(rr_intervals);
            exit(1);
        }
        SALLOC(actual_u8, rr_intervals_capacity, sizeof(uint8_t));
        
        for (WFDB_Time i = 0; i < record_size_u8; i++) {
            printf("Record: %s\n", wfdbfile("atr", (char *)records[i])); // Print record name
            
            // Reset performance metrics for each record
            performance_metrics_init(&metrics);
            
            // Reset for each record
            if (rr_intervals) {
                rr_intervals->length = 0;
            } else {
                fprintf(stderr, "Error: rr_intervals is null\n");
                exit(1);
            }
            #if (DATABASE == MITDB)
            a.name = "atr"; a.stat = WFDB_READ;
            if (annopen((char *)records[i], &a, 1) < 0){
                exit(1);
            }
            
            a0_u32 = NOTQRS;
            t0 = 0;
            while (getann(0, &annot) == 0) {
                a1_u32 = annot.anntyp;
                    t1 = annot.time;

                if (annot.anntyp == RHYTHM && strncmp(annot.aux+1,"(AFIB",5) == 0) {
                    start_afib = 1;
                }
                else if (annot.anntyp == RHYTHM && strncmp(annot.aux+1,"(AFIB",5) != 0) {
                    start_afib = 0;
                }

                /* Does t1 mark a valid interval starting point? */
                if ((a1f_u8[0] && isqrs(a1_u32)) || a1f_u8[a1_u32]) {
                    // Expand capacity if needed
                    if (rr_intervals->length >= rr_intervals_capacity) {
                        rr_intervals_capacity *= 2;
                        SREALLOC(rr_intervals->x, rr_intervals_capacity, sizeof(WFDB_Time));
                        SREALLOC(rr_intervals->time, rr_intervals_capacity, sizeof(WFDB_Time));
                        SREALLOC(actual_u8, rr_intervals_capacity, sizeof(uint8_t));
                        if (!rr_intervals->x || !rr_intervals->time || !actual_u8) {
                            fprintf(stderr, "Error: rr_intervals reallocation failed\n");
                            if (rr_intervals->x) SFREE(rr_intervals->x);
                            if (rr_intervals->time) SFREE(rr_intervals->time);
                            if (actual_u8) SFREE(actual_u8);
                            SFREE(rr_intervals);
                            exit(1);
                        }
                    }
                    
                    // Store RR interval and timestamp
                    rr_intervals->x[rr_intervals->length] = t1 - t0;
                    rr_intervals->time[rr_intervals->length] = t1;
                    
                    // Store AF label
                    if (start_afib) {
                        actual_u8[rr_intervals->length] = 1;
                    } else {
                        actual_u8[rr_intervals->length] = 0;
                    }
                    
                    rr_intervals->length++;
                }
                /* Does t1 mark a valid interval ending point? */
                if ((a0f_u8[0] && isqrs(a1_u32)) || a0f_u8[a1_u32]) {
                    a0_u32 = a1_u32;
                    t0 = t1;
                }
            }
            (void)ungetann(0, &annot); // Unget the last annotation to reset state

            #elif (DATABASE == AFDB)
            a[0].name = "atr"; a[0].stat = WFDB_READ;
            printf("Record: %s\n", wfdbfile("qrsc", (char *)records[i])); // Print record name
                if (annopen((char *)records[i], a, 1) < 0){
                exit(1);
            }
            if (annopen((char *)records[i], a, 1) < 0){
                exit(1);
            }
            
            a0_u32 = NOTQRS;
            while (getann(0, &annot) == 0) {
                a1_u32 = annot.anntyp;

                if (annot.anntyp == RHYTHM && strncmp(annot.aux+1,"(AFIB",5) == 0) {
                    start_afib = 1;
                }
                else if (annot.anntyp == RHYTHM && strncmp(annot.aux+1,"(AFIB",5) != 0) {
                    start_afib = 0;
                }
                    // Expand capacity if needed
                    if (rr_intervals->length >= rr_intervals_capacity) {
                        rr_intervals_capacity *= 2;
                        SREALLOC(actual_u8, rr_intervals_capacity, sizeof(uint8_t));
                        if (!actual_u8) {
                            fprintf(stderr, "Error: rr_intervals reallocation failed\n");
                            if (actual_u8) SFREE(actual_u8);
                            SFREE(rr_intervals);
                            exit(1);
                        }
                    }
                    
                    // Store AF label
                    if (start_afib) {
                        actual_u8[rr_intervals->length] = 1;
                    } else {
                        actual_u8[rr_intervals->length] = 0;
                    }
                    
                    rr_intervals->length++;

                    a0_u32 = a1_u32;
            }
            (void)ungetann(0, &annot); // Unget the last annotation to reset state

            a[1].name = "qrs"; a[1].stat = WFDB_READ;
            if (annopen((char *)records[i], a, 1) < 0){
                exit(1);
            }
            t0 = 0;
            rr_intervals->length = 0; // Reset length to reuse the array
            while (getann(0, &annot) == 0) {
                    t1 = annot.time;
                    // Store RR interval and timestamp
                    // Expand capacity if needed
                    if (rr_intervals->length >= rr_intervals_capacity) {
                        rr_intervals_capacity *= 2;
                        SREALLOC(rr_intervals->x, rr_intervals_capacity, sizeof(WFDB_Time));
                        SREALLOC(rr_intervals->time, rr_intervals_capacity, sizeof(WFDB_Time));
                        if (!rr_intervals->x || !rr_intervals->time) {
                            fprintf(stderr, "Error: rr_intervals reallocation failed\n");
                            if (rr_intervals->x) SFREE(rr_intervals->x);
                            if (rr_intervals->time) SFREE(rr_intervals->time);
                            SFREE(rr_intervals);
                            exit(1);
                        }
                    }
                    rr_intervals->x[rr_intervals->length] = t1 - t0;
                    // Store RR interval and timestamp
                    rr_intervals->time[rr_intervals->length] = t1;
                    rr_intervals->length++;
                    t0 = t1;
            }
            (void)ungetann(1, &annot); // Unget the last annotation to reset state
            #endif
            // Check if we collected any RR intervals
            if (rr_intervals->length == 0) {
                printf("Warning: No RR intervals found for record %s\n", records[i]);
                continue;
            }

            WFDB_Time *y; SALLOC(y, rr_intervals->length, sizeof(WFDB_Time));
            WFDB_Time *xl; SALLOC(xl, rr_intervals->length, sizeof(WFDB_Time));
            WFDB_Time *xh; SALLOC(xh, rr_intervals->length, sizeof(WFDB_Time));
            float *entropy_f32; SALLOC(entropy_f32, rr_intervals->length, sizeof(float));
            WFDB_Time *wv; SALLOC(wv, rr_intervals->length, sizeof(WFDB_Time));
            WFDB_Time *sy; SALLOC(sy, rr_intervals->length, sizeof(WFDB_Time));
            WFDB_Time *predict_u8; SALLOC(predict_u8, rr_intervals->length, sizeof(WFDB_Time));
            
            // Check all allocations
            if (!y || !xl || !xh || !entropy_f32 || !wv || !sy || !predict_u8) {
                fprintf(stderr, "Error: Failed to allocate processing arrays for record %s\n", records[i]);
                if (y) SFREE(y);
                if (xl) SFREE(xl);
                if (xh) SFREE(xh);
                if (entropy_f32) SFREE(entropy_f32);
                if (wv) SFREE(wv);
                if (sy) SFREE(sy);
                if (predict_u8) SFREE(predict_u8);
                continue;
            }

            median_filter(rr_intervals->x, y, rr_intervals->length, 17);

            low_reference_filter(y, xl, rr_intervals->length);

            high_reference_filter(xl, xh, rr_intervals->length);

            // Allocate symbolic array
            symbolic_dynamic(rr_intervals->x, xl, xh, sy, rr_intervals->length);

            // Allocate word sequence array
            word_sequence(sy, wv, rr_intervals->length);

            // After the word_sequence call:
            shannon_entropy_algorithm(wv, entropy_f32, rr_intervals->length);

            memset(predict_u8, 0, rr_intervals->length * sizeof(WFDB_Time));
            for (uint32_t i = 0; i < rr_intervals->length; i++) {
                if (entropy_f32[i] > 0.353) {
                    predict_u8[i] = 1; // Mark as AF episode
                } else {
                    predict_u8[i] = 0; // Not AF episode
                }
            }
            // Update performance metrics with all predictions
            performance_metrics_update(&metrics, actual_u8, predict_u8, rr_intervals->length);
            
            // Calculate performance metrics
            performance_metrics_calculate(&metrics);
            
            // Print performance metrics to console
            performance_metrics_print(&metrics, records[i]);
            
            // Write results to file using the new performance metrics module
            performance_metrics_write_to_file(&metrics, records[i]);
            
            // Export comparison data to file
            export_comparison_to_file(actual_u8, predict_u8, rr_intervals->length, records[i]);
            
            FILE *GNUpipe = NULL,
                 *x_txt = NULL,
                 *y_txt = NULL,
                 *xl_txt = NULL,
                 *xh_txt = NULL,
                 *delta_x_txt = NULL,
                 *wv_txt = NULL,
                 *sy_txt = NULL,
                 *entropy_txt = NULL,
                 *prediction_txt = NULL,
                 *seg_afib_note_txt = NULL;

            GNUpipe = popen("gnuplot -persist", "w");
            if (GNUpipe == NULL) {
                printf("Failed to open GNUpipe\n");
            }

            fprintf(GNUpipe, "set term qt persist\n");
            fprintf(GNUpipe, "set title 'Signal Analysis'\n");
            x_txt = fopen("x_txt", "w");
            y_txt = fopen("y_txt", "w");
            xl_txt = fopen("xl_txt", "w");
            xh_txt = fopen("xh_txt", "w");
            delta_x_txt = fopen("delta_x_txt", "w");
            wv_txt = fopen("wv_txt", "w");
            sy_txt = fopen("sy_txt", "w");
            entropy_txt = fopen("entropy_txt", "w");
            GNUpipe = popen("gnuplot -persist", "w");
            for (WFDB_Time i = 0; i < rr_intervals->length; i++) {
                fprintf(x_txt, "%lld %lld\n", rr_intervals->time[i], rr_intervals->x[i]);
                fflush(x_txt);
                fprintf(y_txt, "%lld %lld\n", rr_intervals->time[i], y[i]);
                fflush(y_txt);
                fprintf(xl_txt, "%lld %lld\n", rr_intervals->time[i], xl[i]);
                fflush(xl_txt);
                fprintf(xh_txt, "%lld %lld\n", rr_intervals->time[i], xh[i]);
                fflush(xh_txt);
                fprintf(delta_x_txt, "%lld %lld\n", rr_intervals->time[i], delta_x[i]);
                fflush(delta_x_txt);
                fprintf(wv_txt, "%lld %lld\n", rr_intervals->time[i], wv[i]);
                fflush(wv_txt);
                fprintf(sy_txt, "%lld %lld\n", rr_intervals->time[i], sy[i]);
                fflush(sy_txt);
                fprintf(entropy_txt, "%lld %f\n", rr_intervals->time[i], entropy_results[i]);
                fflush(entropy_txt);
            }

            Plot each file in its own Gnuplot terminal
            if (GNUpipe) {
                // Set high-resolution terminal with larger window size
                fprintf(GNUpipe, "set terminal qt size 1920,1080 enhanced font 'Arial,12' persist\n");
                fprintf(GNUpipe, "set output\n");
                
                // Set up multiplot layout (2x2 grid)
                fprintf(GNUpipe, "set multiplot layout 3,2 title 'Signal Processing Pipeline Analysis'\n");
                fprintf(GNUpipe, "set grid\n");
                fprintf(GNUpipe, "unset key\n");

                // Plot 1: Original RR uint32_tervals (x)
                fprintf(GNUpipe, "set title 'Original RR Intervals (x)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'RR Interval (ms)'\n");
                fprintf(GNUpipe, "plot 'x_txt' with lines linewidth 2 linecolor rgb 'blue'\n");

                // Plot 2: Median filtered signal (y)
                fprintf(GNUpipe, "set title 'Median Filtered Signal (y)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'Filtered RR (ms)'\n");
                fprintf(GNUpipe, "plot 'y_txt' with lines linewidth 2 linecolor rgb 'red' \n");

                // Plot 3: Low-reference filtered signal (xl)
                fprintf(GNUpipe, "set title 'Low-reference Filtered Signal (xl)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'Low-reference Output'\n");
                fprintf(GNUpipe, "plot 'xl_txt' with lines linewidth 2 linecolor rgb 'green', "
                                 "'xh_txt' with lines linewidth 2 linecolor rgb 'purple'\n");

                // Plot 4: High-reference filtered signal (xh)
                fprintf(GNUpipe, "set title 'High-reference Filtered Signal (xh)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'High-reference Output'\n");
                fprintf(GNUpipe, "plot 'xh_txt' with lines linewidth 2 linecolor rgb 'purple'\n");

                // Plot 5: Delta_RR signal (delta_x)
                fprintf(GNUpipe, "set title 'Delta_RR Signal (delta_x)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'Delta_RR Output'\n");
                fprintf(GNUpipe, "plot 'delta_x_txt' with lines linewidth 2 linecolor rgb 'orange'\n");

                // Plot 6: Symbolic dynamic signal (sy)
                fprintf(GNUpipe, "set title 'Symbol Sequence (sy)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'Symbolic Sequence'\n");
                fprintf(GNUpipe, "plot 'sy_txt' with lines linewidth 2 linecolor rgb 'purple'\n");

                // Plot 7: Word sequence signal (wv)
                fprintf(GNUpipe, "set title 'Word Sequence (wv)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'Word Sequence'\n");
                fprintf(GNUpipe, "plot 'wv_txt' with lines linewidth 2 linecolor rgb 'purple'\n");

                // Plot 8: Shannon entropy signal (se)
                fprintf(GNUpipe, "set title 'Shannon Entropy (se)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'Shannon Entropy'\n");
                fprintf(GNUpipe, "plot 'entropy_txt' with lines linewidth 2 linecolor rgb 'purple', "
                                "0.353 with lines linewidth 2 linecolor rgb 'red' \n");

                // Plot 9: Entropy histogram (variable-width boxes)
                fprintf(GNUpipe, "set title 'Entropy Histogram'\n");
                fprintf(GNUpipe, "set xlabel 'Entropy'\n");
                fprintf(GNUpipe, "set ylabel 'Count'\n");
                fprintf(GNUpipe, "set style fill solid 0.6 noborder\n");
                fprintf(GNUpipe, "plot 'entropy_hist_txt' using (($1+$2)/2):3:(($2-$1)) with boxes lc rgb 'gray'\n");

                // Plot 10: Segmented Atrial Fibrillation Note (seg_afib_note)
                fprintf(GNUpipe, "set title 'Segmented Atrial Fibrillation Note'\n");
                fprintf(GNUpipe, "set xlabel 'Time'\n");
                fprintf(GNUpipe, "set ylabel 'Amplitude'\n");
                fprintf(GNUpipe, "set style fill solid 0.6 noborder\n");
                fprintf(GNUpipe, "plot '%s' with line lc rgb 'red'\n", afib_fn);

                // End multiplot
                fprintf(GNUpipe, "unset multiplot\n");
                fflush(GNUpipe);
            }

            // Clean up per-record resources (do not call wfdbquit here)
            if (x_txt) fclose(x_txt);
            if (y_txt) fclose(y_txt);
            if (xl_txt) fclose(xl_txt);
            if (xh_txt) fclose(xh_txt);
            if (delta_x_txt) fclose(delta_x_txt);
            if (wv_txt) fclose(wv_txt);
            if (sy_txt) fclose(sy_txt);
            if (seg_afib_note_txt) fclose(seg_afib_note_txt);
            if (entropy_txt) fclose(entropy_txt);
            if (GNUpipe) pclose(GNUpipe);

            // Clean up per-record processing arrays only
            SFREE(y);
            SFREE(xl);
            SFREE(xh);
            SFREE(sy);
            SFREE(wv);
            SFREE(entropy_f32);
            SFREE(predict_u8);
        }
        
        // Clean up global resources after all records are processed
        if (rr_intervals) {
            if (rr_intervals->x) SFREE(rr_intervals->x);
            if (rr_intervals->time) SFREE(rr_intervals->time);
            SFREE(rr_intervals);
        }
        if (actual_u8) SFREE(actual_u8);
    }
    wfdbquit();
    exit(0);
}
#endif // SKIP_MAIN
