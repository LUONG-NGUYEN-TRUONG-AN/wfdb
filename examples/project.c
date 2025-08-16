#include "project.h"
#include "test.h"

int PiMap[127] = {  7874,  13495, 18265, 22483, 26290, 29770, 32977, 35952,
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

static int double_compare(const void *a, const void *b) {
    double diff = *(const double*)a - *(const double*)b;
    return (diff > 0) - (diff < 0);
}

void median_filter(const WFDB_Time *input, double *output, const int length, const int flen) {
    // Input validation
    assert(input != NULL && output != NULL);
    assert(length > 0 && flen > 0 && (flen & 1));
    
    int median_idx = flen / 2;
    double *window;
    SUALLOC(window, flen, sizeof(double));
    if (!window) {
        fprintf(stderr, "median_filter: window allocation failed\n");
        return;
    }

    for (int i = 0; i < length; i++) {
        // Fill window with available samples (nearest-neighbor padding)
        for (int j = 0; j < flen; j++) {
            int idx = i - median_idx + j;
            if (idx < 0) idx = 0;
            if (idx >= length) idx = length - 1;
            window[j] = (double)input[idx];
        }
        
        // Sort window using proper double comparison
        qsort(window, flen, sizeof(double), double_compare);
        
        // Store median
        output[i] = window[median_idx];
    }
    
    SFREE(window);
}

/**
 * Low reference filter implementation: y[n] = y[n-1] + x[n] - x[n-16]
 * @param input: Input signal array
 * @param output: Output signal array
 * @param length: Signal length
 */
void low_reference_filter(const double *input, double *output, int length) {
    if (!input || !output || length <= 0) {
        fprintf(stderr, "low_reference_filter: Invalid input parameters\n");
        return;
    }

    const int DELAY_SAMPLES = 16;
    #define SCALE_16 16.0
    for (int n = 0; n < length; n++) {
        double prev_output = (n > 0) ? output[n-1] : 0.0;  // y[n-1], assume y[-1] = 0
        double delayed_input = (n >= DELAY_SAMPLES) ? input[n - DELAY_SAMPLES] : 0.0;  // x[n-16], zero-pad
            
        // Implement the exact difference equation: y[n] = y[n-1] + x[n] - x[n-16]
        output[n] = prev_output + input[n] - delayed_input;
    }
    for (int n = 0; n < length; n++) {
        output[n] = (double)(output[n] / SCALE_16);
    }
}

/**
 * High reference filter implementation: y[n] = (y[n-1] * 2) - y[n-2] + x[n] - x[n-32] - x[n-64] - x[n-96]
 * @param input: Input signal array
 * @param output: Output signal array  
 * @param length: Signal length
 */
void high_reference_filter(const double *input, double *output, int length) {
    if (!input || !output || length <= 0) {
        fprintf(stderr, "high_reference_filter: Invalid input parameters\n");
        return;
    }

    #define DELAY_32 32
    #define DELAY_64 64
    #define DELAY_96 96
    #define SCALE_1024 1024.0

    // Protect against in-place use (input == output)
    const double *src = input;
    double *tmp = NULL;
    if (input == output) {
        SUALLOC(tmp, length, sizeof(double));
        if (!tmp) {
            fprintf(stderr, "high_reference_filter: temp allocation failed for in-place call\n");
            return;
        }
        // Copy input to tmp once
        memcpy(tmp, input, (size_t)length * sizeof(double));
        src = tmp;
    }

    int n = 0;

    // Transient region with zero-padding (n < 96)
    for (; n < length && n < DELAY_96; ++n) {
        double prev_y1 = (n > 0) ? output[n-1] : 0.0;
        double prev_y2 = (n > 1) ? output[n-2] : 0.0;
        double x_n32 = (n >= DELAY_32) ? src[n - DELAY_32] : 0.0;
        double x_n64 = (n >= DELAY_64) ? src[n - DELAY_64] : 0.0;
        double x_n96 = 0.0; // n < 96 here
        output[n] = (prev_y1 * 2.0) - prev_y2 + src[n] - x_n32 - x_n64 + x_n96;
    }

    // Steady-state region: no boundary checks needed (n >= 96)
    for (; n < length; ++n) {
        double prev_y1 = output[n-1];
        double prev_y2 = output[n-2];
        output[n] = (prev_y1 * 2.0) - prev_y2
                    + src[n] - src[n - DELAY_32] - src[n - DELAY_64] + src[n - DELAY_96];
    }

    // Normalization
    for (int i = 0; i < length; ++i) {
        output[i] /= SCALE_1024;
    }

    if (tmp) {
        SFREE(tmp);
    }
}

void symbolic_dynamic(const WFDB_Time *x, const double *xl, const double *xh, int *sy, int len) {
    double *delta_x = NULL;
    SUALLOC(delta_x, len, sizeof(double));
    #define X_DELAY 63   // Delay for x array
    #define XL_DELAY 47  // Delay for xl array
    for (int i = 0; i < len; i++) {
        double x_delayed, xl_delayed;
        
        // Get delayed x value with bounds checking
        if (i >= X_DELAY) {
            x_delayed = (double)x[i - X_DELAY];
        } else {
            x_delayed = (double)x[0];  // Use first value for early samples
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
    for (int i = 0; i < len; i++) {
        double thres1 = (double)(xh[i] / 16.0);
        double thres2 = (double)(xh[i] / 8.0);
        double thres3 = thres1 + thres2;
        double thres4 = (double)(xh[i] / 4.0);
        double thres5 = thres4 + thres1;

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
}

void word_sequence(const int *sy, int *wv, int len) {
    if (!sy || !wv || len <= 0) {
        fprintf(stderr, "word_sequence: Invalid input parameters\n");
        return;
    }
    for (int i = 0; i < len; ++i) {
        int i2 = (i - 2 < 0) ? 0 : (i - 2);
        int i1 = (i - 1 < 0) ? 0 : (i - 1);
        int s2 = sy[i2];
        int s1 = sy[i1];
        int s0 = sy[i];
        // sy expected in [0..9]; if broader inputs occur, consider clamping to [0..15].
        wv[i] = (s2 << 8) | (s1 << 4) | s0;
    }
}

/**
 * Shannon entropy algorithm following the flowchart logic
 * Uses occurrence counting and proper PiMap derivative calculation
 */
void shannon_entropy_algorithm(const int *inputs, double *results, int input_len, int window_size) {
    if (!inputs || !results || input_len <= 0 || window_size <= 0) {
        fprintf(stderr, "shannon_entropy_algorithm: Invalid input parameters\n");
        return;
    }
    
    if (input_len < window_size) {
        fprintf(stderr, "shannon_entropy_algorithm: Input length (%d) less than window size (%d)\n", 
                input_len, window_size);
        return;
    }
    
    #define max_wv 2457 // (9 * 256 + 9 * 16 + 9) - Maximum possible word value
    #define PIMAP_SIZE 127
    #define ENTROPY_SCALE 127000000.0

    int *sh;
    double *SH;
    SUALLOC(sh, input_len, sizeof(int));
    SUALLOC(SH, input_len, sizeof(double));
    
    if (!sh || !SH) {
        fprintf(stderr, "shannon_entropy_algorithm: Memory allocation failed\n");
        if (sh) SFREE(sh);
        if (SH) SFREE(SH);
        return;
    }

    memset(sh, 0, input_len * sizeof(int));
    memset(SH, 0, input_len * sizeof(double));
    memset(results, 0, input_len * sizeof(double));

    int *ntwv; // Word occurrence counts (following flowchart notation)
    SUALLOC(ntwv, max_wv, sizeof(int));
    if (!ntwv) {
        fprintf(stderr, "shannon_entropy_algorithm: ntwv allocation failed\n");
        SFREE(sh);
        SFREE(SH);
        return;
    }
    
    memset(ntwv, 0, max_wv * sizeof(int)); // Initialize to zero
    int k = 0; // Count of unique words in current window
    int te_in, te_out; // Temporary variables for PiMap calculations
    
    for (int i = 0; i < window_size && i < input_len; i++) {
        int word_val = inputs[i];
        if (word_val >= 0 && word_val < max_wv) {
            if (ntwv[word_val] == 0) {
                k++; // New unique word
            }
            ntwv[word_val]++; // Increment occurrence count
        }
    }

    for (int pos = window_size; pos < input_len - 1; pos++) {
        

        int wvn = inputs[pos];           // word entering (wv_n)
        int wvn_127 = inputs[pos - 127]; // word leaving (wv_{n-127})

        if (wvn == wvn_127) {
            // No change in word composition
            sh[pos] = sh[pos-1];  // sh'n = sh'_{n-1}
        }
        else {
            te_in = ntwv[wvn];      // Current count before increment
            te_out = ntwv[wvn_127]; // Current count before decrement
            
            ntwv[wvn_127]--;        // Decrement leaving word
            ntwv[wvn]++;            // Increment entering word
            
            if (ntwv[wvn_127] < 0) ntwv[wvn_127] = 0; // Prevent negative
        }
        // Get updated counts for PiMap indexing
        int ntwvn = ntwv[wvn];          // Updated count for entering word
        int ntwvn_127 = ntwv[wvn_127];  // Updated count for leaving word

        // Apply flowchart formula with bounds checking
        sh[pos] = sh[pos-1] + 
                (PiMap[ntwvn] - PiMap[te_in]) + 
                (PiMap[ntwvn_127] - PiMap[te_out]);
                if (te_in == 0) {  // New word entering
            if ((te_out > 1) || (te_out == 0)) {
                k++;
            }
        } else {  // Existing word count increased
            if ((ntwvn_127 == 0) && (te_out == 1)) {
                k--;  // Word completely left the window
            }
        }
        SH[pos] = (double)(k * sh[pos]) / 127000000.0;
        results[pos] = SH[pos];
    }

    // Fill early positions with appropriate values
    for (int i = 0; i < window_size && i < input_len; i++) {
        results[i] = (window_size < input_len) ? results[window_size] : 0.0;
    }

    // Clean up memory following project conventions
    SFREE(ntwv);
    SFREE(sh);
    SFREE(SH);
}

/**
 * Build histogram (frequency count) for entropy_results.
 * - Computes min/max internally.
 * - Allocates counts[num_bins] and edges[num_bins+1] via SALLOC; caller must SFREE both.
 * Returns 0 on success, -1 on error.
 */
static int entropy_histogram(const double *arr, int len, int num_bins,
                             int **counts_out, double **edges_out,
                             double *min_out, double *max_out) {
    if (!arr || len <= 0 || num_bins <= 0 || !counts_out || !edges_out || !min_out || !max_out) {
        return -1;
    }

    // Compute min/max
    double mn = arr[0], mx = arr[0];
    for (int i = 1; i < len; ++i) {
        if (arr[i] < mn) mn = arr[i];
        if (arr[i] > mx) mx = arr[i];
    }
    // Avoid zero-width range
    if (mx == mn) {
        mx = mn + 1e-9;
    }

    double *edges; SALLOC(edges, (size_t)(num_bins + 1), sizeof(double));
    int *counts;   SALLOC(counts, (size_t)num_bins, sizeof(int));
    if (!edges || !counts) {
        if (edges) SFREE(edges);
        if (counts) SFREE(counts);
        return -1;
    }
    memset(counts, 0, num_bins * sizeof(int));

    const double width = (mx - mn) / (double)num_bins;
    for (int i = 0; i <= num_bins; ++i) {
        edges[i] = mn + width * (double)i;
    }

    // Accumulate counts
    for (int i = 0; i < len; ++i) {
        int idx = (int)((arr[i] - mn) / width);
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

void main() {
    #if (DATABASE == MITDB)
        setwfdb("/home/an/physionet.org/files/mitdb/1.0.0");
        // const char *records[] = {"201"};
        // const char *records[] = {"202"};
        // const char *records[] = {"203"};
        // const char *records[] = {"210"};
        // const char *records[] = {"217"};
        // const char *records[] = {"219"};
        // const char *records[] = {"221"};
        // const char *records[] = {"222"};
        // const char *records[] = {"200", "201", "202", "203", "210", "217", "219", "221", "222"};
        int record_size = sizeof(records) / sizeof(records[0]);
    #elif (DATABASE == AFDB)
        setwfdb("/home/an/physionet.org/files/afdb/1.0.0");
    #endif

    // {'N', 'L', 'R', 'a', 'V', 'F', 'J', 'A', 'S', 'E', 'j', '/', 'Q'};
    const char **record = records;
    const int a0f[ACMAX+1] = {  /* annotation filter array for interval start points */
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
    const int a1f[ACMAX+1] = {  /* annotation filter array for interval start points */
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
        int *segment_afib_note = NULL;
        WFDB_Anninfo a[1];
        WFDB_Annotation annot;
        static WFDB_Time *x = NULL, *time = NULL, t0, t1;
        int previous_annot_valid = 0, 
            a0, 
            a1,
            Number_RR = 0,
            start_afib = 0,
            TP = 0,
            FP = 0,
            FN = 0,
            TN = 0;
        double Se = 0.0, 
               Sp = 0.0, 
               PPV = 0.0,
               ACC = 0.0;
        SALLOC(x, 1, sizeof(WFDB_Time));
        SALLOC(time, 1, sizeof(WFDB_Time));
        SALLOC(segment_afib_note, 1, sizeof(int));
        FILE *GNUpipe = NULL,
             *x_txt = NULL,
             *y_txt = NULL,
             *xl_txt = NULL,
             *xh_txt = NULL,
             *delta_x_txt = NULL,
             *wv_txt = NULL,
             *sy_txt = NULL,
             *entropy_results_txt = NULL,
             *prediction_txt = NULL,
             *seg_afib_note_txt = NULL;

        GNUpipe = popen("gnuplot -persist", "w");
        if (GNUpipe == NULL) {
            printf("Failed to open GNUpipe\n");
        }

        fprintf(GNUpipe, "set term qt persist\n");
        fprintf(GNUpipe, "set title 'Signal Analysis'\n");
        a[0].name = "atr"; a[0].stat = WFDB_READ;
        for (int i = 0; i < record_size; i++) {
            printf("Record: %s\n", records[i]); // Print record name
            if (annopen((char *)records[i], a, 1) < 0){
                exit(1);
            }
            a0 = NOTQRS;
            t0 = 0;
            while (getann(0, &annot) == 0) {
                a1 = annot.anntyp;
                t1 = annot.time;

                if (annot.anntyp==RHYTHM && strncmp(annot.aux+1,"(AFIB",5) == 0) {
                    start_afib = 1;
                }
                else if (annot.anntyp==RHYTHM && strncmp(annot.aux+1,"(AFIB",5) != 0) {
                    start_afib = 0;
                }

                /* Does t1 mark a valid interval starting point? */
                if ((a1f[0] && isqrs(a1)) || a1f[a1]) {
                    Number_RR++;
                    SREALLOC(x, Number_RR, sizeof(WFDB_Time));
                    x[Number_RR - 1] = t1 - t0;

                    SREALLOC(segment_afib_note, Number_RR, sizeof(int));
                    SREALLOC(time, Number_RR, sizeof(WFDB_Time));
                    if (start_afib) {
                        segment_afib_note[Number_RR - 1] = 1;
                    } else {
                        segment_afib_note[Number_RR - 1] = 0;
                    }
                    time[Number_RR - 1] = t1;
                }
                /* Does t1 mark a valid interval starting point? */
                if ((a0f[0] && isqrs(a1)) || a0f[a1]) {
                    a0 = a1;
                    t0 = t1;
                }
            }
            double *y; SALLOC(y, Number_RR, sizeof(double));
            double *xl; SALLOC(xl, Number_RR, sizeof(double));
            double *xh; SALLOC(xh, Number_RR, sizeof(double));
            double *entropy_results; SALLOC(entropy_results, Number_RR, sizeof(double));
            int *wv; SALLOC(wv, Number_RR, sizeof(int));
            int *sy; SALLOC(sy, Number_RR, sizeof(int));
            int *prediction; SALLOC(prediction, Number_RR + DELAY, sizeof(int));
            int *predict_episode; SALLOC(predict_episode, Number_RR, sizeof(int));

            median_filter(x, y, Number_RR, 17);

            low_reference_filter(y, xl, Number_RR);

            high_reference_filter(xl, xh, Number_RR);

            // Allocate symbolic array
            symbolic_dynamic(x, xl, xh, sy, Number_RR);

            // Allocate word sequence array
            word_sequence(sy, wv, Number_RR);

            // After the word_sequence call:
            shannon_entropy_algorithm(wv, entropy_results, Number_RR, 127);

            memset(prediction, 0, Number_RR * sizeof(int));
            for (int i = 0; i < Number_RR; i++) {
                if (entropy_results[i] > 0.353) {
                    prediction[i] = 1; // Mark as AF episode
                } else {
                    prediction[i] = 0; // Not AF episode
                }
            }
            for (int i = 0; i < Number_RR; i++) {
                if ((segment_afib_note[i] == 1) && (prediction[i] == 1)) {
                    TP++;
                } else if ((segment_afib_note[i] == 0) && (prediction[i] == 1)) {
                    FP++;
                } else if ((segment_afib_note[i] == 1) && (prediction[i] == 0)) {
                    FN++;
                } else if ((segment_afib_note[i] == 0) && (prediction[i] == 0)) {
                    TN++;
                }
            }
            Se = (double)TP / (TP + FN);
            Sp = (double)TN / (TN + FP);
            PPV = (double)TP / (TP + FP);
            ACC = (double)(TP + TN) / (TP + TN + FP + FN);
            printf("TP: %d, FP: %d, FN: %d, TN: %d\n", TP, FP, FN, TN);
            printf("Sensitivity (Se): %.2f, Specificity (Sp): %.2f, PPV: %.2f, ACC: %.2f\n",
                   Se, Sp, PPV, ACC);
            (void)ungetann(0, &annot); // Unget the last annotation to reset state
            
            x_txt = fopen("x_txt", "w");
            y_txt = fopen("y_txt", "w");
            xl_txt = fopen("xl_txt", "w");
            xh_txt = fopen("xh_txt", "w");
            delta_x_txt = fopen("delta_x_txt", "w");
            wv_txt = fopen("wv_txt", "w");
            sy_txt = fopen("sy_txt", "w");
            entropy_results_txt = fopen("entropy_results_txt", "w");
            GNUpipe = popen("gnuplot -persist", "w");
            for (int i = 0; i < Number_RR; i++) {
                fprintf(x_txt, "%lld %lld\n", time[i], x[i]);
                fflush(x_txt);
                fprintf(y_txt, "%lld %.0f\n", time[i], y[i]);
                fflush(y_txt);
                fprintf(xl_txt, "%lld %.0f\n", time[i], xl[i]);
                fflush(xl_txt);
                fprintf(xh_txt, "%lld %.0f\n", time[i], xh[i]);
                fflush(xh_txt);
                // fprintf(delta_x_txt, "%lld %f\n", time[i], delta_x[i]);
                // fflush(delta_x_txt);
                fprintf(wv_txt, "%lld %d\n", time[i], wv[i]);
                fflush(wv_txt);
                fprintf(sy_txt, "%lld %d\n", time[i], sy[i]);
                fflush(sy_txt);
                fprintf(entropy_results_txt, "%lld %f\n", time[i], entropy_results[i]);
                fflush(entropy_results_txt);
            }

            // Plot each file in its own Gnuplot terminal
            if (GNUpipe) {
                // Set high-resolution terminal with larger window size
                fprintf(GNUpipe, "set terminal qt size 3000,3000 enhanced font 'Arial,12' persist\n");
                fprintf(GNUpipe, "set output\n");
                
                // Set up multiplot layout (2x2 grid)
                fprintf(GNUpipe, "set multiplot layout 4,2 title 'Signal Processing Pipeline Analysis'\n");
                fprintf(GNUpipe, "set grid\n");

                // Plot 1: Original RR intervals (x)
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
                fprintf(GNUpipe, "plot 'xl_txt' with lines linewidth 2 linecolor rgb 'green'\n");

                // Plot 4: High-reference filtered signal (xh)
                fprintf(GNUpipe, "set title 'High-reference Filtered Signal (xh)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'High-reference Output'\n");
                fprintf(GNUpipe, "plot 'xh_txt' with lines linewidth 2 linecolor rgb 'purple'\n");

                // Plot 5: Delta_RR signal (delta_x)
                // fprintf(GNUpipe, "set title 'Delta_RR Signal (delta_x)'\n");
                // fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                // fprintf(GNUpipe, "set ylabel 'Delta_RR Output'\n");
                // fprintf(GNUpipe, "plot 'delta_x_txt' with lines linewidth 2 linecolor rgb 'orange'\n");

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
                fprintf(GNUpipe, "plot 'entropy_results_txt' with lines linewidth 2 linecolor rgb 'purple', "
                                "0.353 with lines linewidth 2 linecolor rgb 'red' \n");

                // Plot 9: Entropy histogram (variable-width boxes)
                fprintf(GNUpipe, "set title 'Entropy Histogram'\n");
                fprintf(GNUpipe, "set xlabel 'Entropy'\n");
                fprintf(GNUpipe, "set ylabel 'Count'\n");
                fprintf(GNUpipe, "set style fill solid 0.6 noborder\n");
                fprintf(GNUpipe, "plot 'entropy_hist_txt' using (($1+$2)/2):3:(($2-$1)) with boxes lc rgb 'gray'\n");

                // Plot 10: Segmented Atrial Fibrillation Note (seg_afib_note)
                // fprintf(GNUpipe, "set title 'Segmented Atrial Fibrillation Note'\n");
                // fprintf(GNUpipe, "set xlabel 'Time'\n");
                // fprintf(GNUpipe, "set ylabel 'Amplitude'\n");
                // fprintf(GNUpipe, "set style fill solid 0.6 noborder\n");
                // fprintf(GNUpipe, "plot '%s' with line lc rgb 'red'\n", afib_fn);

                // End multiplot
                fprintf(GNUpipe, "unset multiplot\n");
                fprintf(GNUpipe, "unset key\n");
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
            if (entropy_results_txt) fclose(entropy_results_txt);
            if (GNUpipe) pclose(GNUpipe);
            
            
            SFREE(x);
            SFREE(y);
            SFREE(xl);
            SFREE(xh);
            // SFREE(delta_x);
            SFREE(sy);
            SFREE(wv);
            SFREE(entropy_results);
            SFREE(segment_afib_note);
            SFREE(time);
        }
    }
    wfdbquit();
    exit(0);
}
