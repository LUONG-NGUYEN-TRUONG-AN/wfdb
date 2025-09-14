#include "project.h"
#include "test.h"

uint32_t PiMap[127] = {0, 7874, 13495, 18265, 22483, 26290, 29770, 32977, 35952,
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
                       9519, 7965, 6398, 4818, 3225, 1619};

/**
 * Initialize a new rr_intervals structure with given initial capacity
 * @param initial_capacity: Initial capacity for the arrays
 * @return: Pointer to initialized rr_intervals_t structure, or NULL on failure
 */
rr_intervals_t *rr_intervals_init(size_t initial_capacity)
{
    rr_intervals_t *rr_intervals = NULL;

    // Allocate the structure
    SUALLOC(rr_intervals, 1, sizeof(rr_intervals_t));
    if (!rr_intervals)
    {
        fprintf(stderr, "Error: rr_intervals allocation failed\n");
        return NULL;
    }

    // Initialize fields
    rr_intervals->length = 0;

    // Allocate arrays
    SUALLOC(rr_intervals->x, initial_capacity, sizeof(WFDB_Time));
    SUALLOC(rr_intervals->time, initial_capacity, sizeof(WFDB_Time));
    SUALLOC(rr_intervals->type, initial_capacity, sizeof(char));

    if (!rr_intervals->x || !rr_intervals->time || !rr_intervals->type)
    {
        fprintf(stderr, "Error: rr_intervals arrays allocation failed\n");
        rr_intervals_free(rr_intervals);
        return NULL;
    }

    return rr_intervals;
}

/**
 * Free an rr_intervals structure and all its allocated memory
 * @param rr_intervals: Pointer to rr_intervals_t structure to free
 */
void rr_intervals_free(rr_intervals_t *rr_intervals)
{
    if (rr_intervals)
    {
        if (rr_intervals->x)
            free(rr_intervals->x);
        if (rr_intervals->time)
            free(rr_intervals->time);
        if (rr_intervals->type)
            free(rr_intervals->type);
        free(rr_intervals);
    }
}

/**
 * Expand the capacity of rr_intervals arrays and actual_u8 array
 * @param rr_intervals: Pointer to rr_intervals_t structure
 * @param new_capacity: New capacity for the arrays
 * @param actual_u8: Pointer to actual_u8 array pointer (will be reallocated)
 * @return: 0 on success, -1 on failure
 */
int rr_intervals_expand(rr_intervals_t *rr_intervals, size_t new_capacity, uint8_t **actual_u8)
{
    if (!rr_intervals || !actual_u8)
    {
        return -1;
    }

    // Reallocate arrays
    SREALLOC(rr_intervals->x, new_capacity, sizeof(WFDB_Time));
    SREALLOC(rr_intervals->time, new_capacity, sizeof(WFDB_Time));
    SREALLOC(rr_intervals->type, new_capacity, sizeof(char));
    SREALLOC(*actual_u8, new_capacity, sizeof(uint8_t));

    if (!rr_intervals->x || !rr_intervals->time || !rr_intervals->type || !*actual_u8)
    {
        fprintf(stderr, "Error: rr_intervals reallocation failed\n");
        return -1;
    }

    return 0;
}

void main()
{
#if (DATABASE == MITDB)
    setwfdb("https://physionet.org/files/mitdb/1.0.0/");
    // Complete MITDB records array from RECORDS file
    // const char *records[] = {
    //     "100", "101", "102", "103", "104", "105", "106", "107", "108", "109",
    //     "111", "112", "113", "114", "115", "116", "117", "118", "119", "121",
    //     "122", "123", "124", "200", "201", "202", "203", "205", "207", "208",
    //     "209", "210", "212", "213", "214", "215", "217", "219", "220", "221",
    //     "222", "223", "228", "230", "231", "232", "233", "234"
    // };
    const char *records[] = {"201",
                             "202",
                             "203",
                             "210",
                             "217",
                             "219",
                             "221",
                             "222"};
    // const char *records[] = {"201"};
    // const char *records[] = {"202"};
    // const char *records[] = {"203"};
    // const char *records[] = {"210"};
    // const char *records[] = {"217"};
    // const char *records[] = {"219"};
    // const char *records[] = {"221"};
    // const char *records[] = {"222"};
#elif (DATABASE == AFDB)
    setwfdb("https://physionet.org/files/afdb/1.0.0/");
    // const char *records[] = {"00735"};
    // const char *records[] = {"04048"};
    // const char *records[] = {"00735", "03665", "04015", "04043", "04048", "04126", "04746", "04908", "04936", "05091", "05121", "05261", "06426", "06453", "06995", "07162", "07859", "07879", "07910", "08215", "08219", "08378", "08405", "08434", "08455"};
    const char *records[] = {
        "08455"};
#elif (DATABASE == LTAFDB)
    setwfdb("https://physionet.org/files/ltafdb/1.0.0/");
    const char *records[] = {
        "00", "01", "03", "05", "06", "07", "08", "10",
        "100", "101", "102", "103", "104", "105", "11",
        "110", "111", "112", "113", "114", "115", "116",
        "117", "118", "119", "12", "120", "121", "122",
        "13", "15", "16", "17", "18", "19", "20",
        "200", "201", "202", "203", "204", "205", "206",
        "207", "208", "21", "22", "23", "24", "25", "26",
        "28", "30", "32", "33", "34", "35", "37", "38",
        "39", "42", "43", "44", "45", "47", "48", "49",
        "51", "53", "54", "55", "56", "58", "60", "62",
        "64", "65", "68", "69", "70", "71", "72", "74", "75"};
#endif
    uint8_t record_size_u8 = sizeof(records) / sizeof(records[0]);
    // {'N', 'L', 'R', 'a', 'V', 'F', 'J', 'A', 'S', 'E', 'j', '/', 'Q'};
    const char **record = records;

    // Reset results file at the beginning of each project run
    performance_metrics_reset_file();

    const uint8_t a0f_u8[ACMAX + 1] = {
        /* annotation filter array for uint32_terval start pouint32_ts */
        1, 1, 1, 1, 1, /* 0 - 4 */
        1, 1, 1, 1, 1, /* 5 - 9 */
        1, 1, 1, 1, 0, /* 10 - 14 */
        0, 0, 0, 0, 0, /* 15 - 19 */
        0, 0, 0, 0, 0, /* 20 - 24 */
        0, 0, 0, 0, 0, /* 25 - 29 */
        0, 0, 0, 0, 0, /* 30 - 34 */
        0, 0, 0, 0, 0, /* 35 - 39 */
        0, 0, 0, 0, 0, /* 40 - 44 */
        0, 0, 0, 0, 0  /* 45 - 49 */
    };
    const uint8_t a1f_u8[ACMAX + 1] = {
        /* annotation filter array for uint32_terval start pouint32_ts */
        1, 1, 1, 1, 1, /* 0 - 4 */
        1, 1, 1, 1, 1, /* 5 - 9 */
        1, 1, 1, 1, 0, /* 10 - 14 */
        0, 0, 0, 0, 0, /* 15 - 19 */
        0, 0, 0, 0, 0, /* 20 - 24 */
        0, 0, 0, 0, 0, /* 25 - 29 */
        0, 0, 0, 0, 0, /* 30 - 34 */
        0, 0, 0, 0, 0, /* 35 - 39 */
        0, 0, 0, 0, 0, /* 40 - 44 */
        0, 0, 0, 0, 0  /* 45 - 49 */
    };

    if (record != NULL)
    {
#if (DATABASE == MITDB)
        WFDB_Anninfo a;
        WFDB_Annotation annot;
#elif (DATABASE == AFDB) || (DATABASE == LTAFDB)
        WFDB_Anninfo a[2];
        WFDB_Annotation annot[2];
#endif
        WFDB_Time t0, t1;
        uint8_t *actual_u8 = NULL;
        double sps, tps, tpm, tph, n;
        int sprec, mprec, hprec;
        WFDB_Time t0_u32 = 0,
                  t1_u32 = 0,
                  a0_u32 = 0,
                  a1_u32 = 0;
        unsigned char start_afib = 0;
#if (BUILDCONFIG == BUILDCONFIG_DEBUG)
        performance_metrics_t metrics;
        performance_metrics_init(&metrics);

        // Array to store all performance metrics for averaging
        performance_metrics_t *all_metrics = NULL;
        SUALLOC(all_metrics, record_size_u8, sizeof(performance_metrics_t));
        if (!all_metrics)
        {
            fprintf(stderr, "Error: all_metrics allocation failed\n");
            exit(1);
        }
#endif
        size_t rr_intervals_capacity = 1000; // Initial capacity
        rr_intervals_t *rr_intervals = rr_intervals_init(rr_intervals_capacity);
        if (!rr_intervals)
        {
            exit(1);
        }
        SALLOC(actual_u8, rr_intervals_capacity, sizeof(uint8_t));

        for (WFDB_Time i = 0; i < record_size_u8; i++)
        {
#if (BUILDCONFIG == BUILDCONFIG_DEBUG)
            if (record == NULL)
            {
                exit(1);
            }

            // Reset performance metrics for each record
            performance_metrics_init(&metrics);
#endif
            // Reset for each record
            if (rr_intervals)
            {
                rr_intervals->length = 0;
            }
            else
            {
                fprintf(stderr, "Error: rr_intervals is null\n");
                exit(1);
            }
#if (DATABASE == MITDB)
            a.name = "atr";
            a.stat = WFDB_READ;

            if (annopen((char *)records[i], &a, 1) < 0)
            {
                exit(1);
            }
#if (BUILDCONFIG == BUILDCONFIG_DEBUG)
            if ((sps = sampfreq((char *)records[i])) < 0.)
                (void)setsampfreq(sps = WFDB_DEFFREQ);

            if ((tps = getiaorigfreq(0)) < sps)
                tps = sps;

            sprec = 3;
            n = 1000;
            while (n < tps)
            {
                sprec++;
                n *= 10;
            }

            tpm = 60.0 * tps;
            mprec = 5;
            n = 100000;
            while (n < tpm)
            {
                mprec++;
                n *= 10;
            }

            tph = 60.0 * tpm;
            hprec = 7;
            n = 10000000;
            while (n < tph)
            {
                hprec++;
                n *= 10;
            }
#endif
            a0_u32 = NOTQRS;
            t0 = 0;
            while (getann(0, &annot) == 0)
            {
                a1_u32 = annot.anntyp;

                if (tps == sps)
                {
                    t1 = annot.time;
                }
                else
                {
                    t1 = annot.time * sps / tps + 0.5;
                    if (t1 > annot.time * sps / tps + 0.5)
                        t1--;
                }
                if (annot.anntyp == RHYTHM && strncmp(annot.aux + 1, "(AFIB", 5) == 0)
                {
                    start_afib = 1;
                }
                else if (annot.anntyp == RHYTHM && strncmp(annot.aux + 1, "(AFIB", 5) != 0)
                {
                    start_afib = 0;
                }

                /* Does t1 mark a valid interval starting point? */
                if ((a1f_u8[0] && isqrs(a1_u32)) || a1f_u8[a1_u32])
                {
                    // Expand capacity if needed
                    if (rr_intervals->length >= rr_intervals_capacity)
                    {
                        rr_intervals_capacity *= 2;
                        if (rr_intervals_expand(rr_intervals, rr_intervals_capacity, &actual_u8) != 0)
                        {
                            rr_intervals_free(rr_intervals);
                            if (actual_u8)
                                SFREE(actual_u8);
                            exit(1);
                        }
                    }

                    // Store RR interval and timestamp
                    rr_intervals->x[rr_intervals->length] = t1 - t0;
                    rr_intervals->time[rr_intervals->length] = t1;
                    rr_intervals->type[rr_intervals->length] = a1_u32;

                    // Store AF label
                    if (start_afib)
                    {
                        actual_u8[rr_intervals->length] = 1;
                    }
                    else
                    {
                        actual_u8[rr_intervals->length] = 0;
                    }

                    rr_intervals->length++;
                }
                /* Does t1 mark a valid interval ending point? */
                if ((a0f_u8[0] && isqrs(a1_u32)) || a0f_u8[a1_u32])
                {
                    a0_u32 = a1_u32;
                    t0 = t1;
                }
            }
            (void)ungetann(0, &annot); // Unget the last annotation to reset state

#elif (DATABASE == AFDB) || (DATABASE == LTAFDB)
            a[0].name = "qrs";
            a[0].stat = WFDB_READ;
            a[1].name = "atr";
            a[1].stat = WFDB_READ;
            if (annopen((char *)records[i], a, 2) < 0)
            {
                exit(2);
            }
#if (BUILDCONFIG == BUILDCONFIG_DEBUG)
            if ((sps = sampfreq((char *)records[i])) < 0.)
                (void)setsampfreq(sps = WFDB_DEFFREQ);

            if ((tps = getiaorigfreq(0)) < sps)
                tps = sps;

            sprec = 3;
            n = 1000;
            while (n < tps)
            {
                sprec++;
                n *= 10;
            }

            tpm = 60.0 * tps;
            mprec = 5;
            n = 100000;
            while (n < tpm)
            {
                mprec++;
                n *= 10;
            }

            tph = 60.0 * tpm;
            hprec = 7;
            n = 10000000;
            while (n < tph)
            {
                hprec++;
                n *= 10;
            }
#endif
            a0_u32 = NOTQRS;
            t0 = 0;
            start_afib = 0; // Global AF state

            // Process both annotation streams simultaneously
            int qrs_available = 1, rhythm_available = 1;
            WFDB_Annotation next_qrs, next_rhythm;

            // Prime the annotation readers
            qrs_available = (getann(0, &next_qrs) == 0);
            rhythm_available = (getann(1, &next_rhythm) == 0);

            while (qrs_available || rhythm_available)
            {
                // Process the annotation with the earlier timestamp
                if (rhythm_available && (!qrs_available || next_rhythm.time <= next_qrs.time))
                {
                    // Process rhythm annotation to update AF state
                    if (next_rhythm.anntyp == RHYTHM && next_rhythm.aux && next_rhythm.aux[0] > 0)
                    {
                        if (strncmp((char *)(next_rhythm.aux + 1), "(AFIB", 5) == 0)
                        {
                            start_afib = 1;
                        }
                        else if (strncmp((char *)(next_rhythm.aux + 1), "(N", 2) == 0 ||
                                 strncmp((char *)(next_rhythm.aux + 1), "(AFL", 4) == 0 ||
                                 strncmp((char *)(next_rhythm.aux + 1), "(VT", 3) == 0)
                        {
                            start_afib = 0;
                        }
                    }
                    rhythm_available = (getann(1, &next_rhythm) == 0);
                }
                else if (qrs_available)
                {
                    // Process QRS annotation using current AF state
                    annot[0] = next_qrs;
                    a1_u32 = annot[0].anntyp;
                    if (tps == sps)
                    {
                        t1 = annot[0].time;
                    }
                    else
                    {
                        t1 = annot[0].time * sps / tps + 0.5;
                        if (t1 > annot[0].time * sps / tps + 0.5)
                            t1--;
                    }
                    /* Does t1 mark a valid interval starting point? */
                    if ((a1f_u8[0] && isqrs(a1_u32)) || a1f_u8[a1_u32])
                    {
                        // Expand capacity if needed
                        if (rr_intervals->length >= rr_intervals_capacity)
                        {
                            rr_intervals_capacity *= 2;
                            if (rr_intervals_expand(rr_intervals, rr_intervals_capacity, &actual_u8) != 0)
                            {
                                rr_intervals_free(rr_intervals);
                                if (actual_u8)
                                    SFREE(actual_u8);
                                exit(1);
                            }
                        }

                        // Store RR interval and timestamp
                        rr_intervals->x[rr_intervals->length] = t1 - t0;
                        rr_intervals->time[rr_intervals->length] = t1;
                        rr_intervals->type[rr_intervals->length] = a1_u32;

                        // Store current AF state
                        actual_u8[rr_intervals->length] = start_afib;

                        rr_intervals->length++;
                    }

                    /* Does t1 mark a valid interval ending point? */
                    if ((a0f_u8[0] && isqrs(a1_u32)) || a0f_u8[a1_u32])
                    {
                        a0_u32 = a1_u32;
                        t0 = t1;
                    }

                    qrs_available = (getann(0, &next_qrs) == 0);
                }
            }
            (void)ungetann(0, &annot[0]);
            (void)ungetann(1, &annot[1]);
            (void)wfdb_anclose();

#endif
            // Check if we collected any RR intervals
            if (rr_intervals->length == 0)
            {
                fprintf(stderr, "Warning: No RR intervals found for record %s\n", records[i]);
                continue;
            }

            int32_t y[rr_intervals->length];
            int32_t xl[rr_intervals->length];
            int32_t xh[rr_intervals->length];
            float entropy_f32[rr_intervals->length];
            WFDB_Time wv[rr_intervals->length];
            uint8_t sy[rr_intervals->length];
            int8_t predict_u8[rr_intervals->length];

            // Check all allocations

            median_filter(rr_intervals->x, y, rr_intervals->length);

            low_reference_filter(y, xl, rr_intervals->length);

            high_reference_filter(xl, xh, rr_intervals->length);

            // Allocate symbolic array
            symbolic_dynamic(rr_intervals->x, xl, xh, sy, rr_intervals->length);

            // Allocate word sequence array
            word_sequence(sy, wv, rr_intervals->length);

            // After the word_sequence call:
            shannon_entropy_algorithm(wv, entropy_f32, rr_intervals->length);

            memset(predict_u8, 0, rr_intervals->length * sizeof(int8_t));
            for (uint32_t i = 0; i < rr_intervals->length; i++)
            {
                if (entropy_f32[i] >= 0.353)
                {
                    predict_u8[i] = 1; // Mark as AF episode
                }
                else
                {
                    predict_u8[i] = 0; // Not AF episode
                }
            }
#if (BUILDCONFIG == BUILDCONFIG_DEBUG)
            // Update performance metrics with all predictions
            performance_metrics_update(&metrics, actual_u8, predict_u8, rr_intervals->length);

            // Calculate performance metrics
            performance_metrics_calculate(&metrics);

            // Store metrics for this record in the array for averaging
            all_metrics[i] = metrics;

            // Print performance metrics to console
            performance_metrics_print(&metrics, records[i]);

            // Write results to file using the new performance metrics module
            performance_metrics_write_to_file(&metrics, records[i]);

            // Export performance metrics to CSV file
            export_metrics_to_csv(&metrics, records[i]);
            FILE *GNUpipe = NULL,
                 *x_txt = NULL,
                 *y_txt = NULL,
                 *xl_txt = NULL,
                 *xh_txt = NULL,
                 *wv_txt = NULL,
                 *sy_txt = NULL,
                 *entropy_txt = NULL,
                 *prediction_txt = NULL,
                 *seg_afib_note_txt = NULL;

            ensure_gen_directory();
            x_txt = fopen("gen/x_txt", "w");
            y_txt = fopen("gen/y_txt", "w");
            xl_txt = fopen("gen/xl_txt", "w");
            xh_txt = fopen("gen/xh_txt", "w");
            wv_txt = fopen("gen/wv_txt", "w");
            sy_txt = fopen("gen/sy_txt", "w");
            entropy_txt = fopen("gen/entropy_txt", "w");
            for (WFDB_Time i = 0; i < rr_intervals->length; i++)
            {
                fprintf(x_txt, "%lld %lld\n", rr_intervals->time[i], rr_intervals->x[i]);
                fflush(x_txt);
                fprintf(y_txt, "%lld %d\n", rr_intervals->time[i], y[i]);
                fflush(y_txt);
                fprintf(xl_txt, "%lld %d\n", rr_intervals->time[i], xl[i]);
                fflush(xl_txt);
                fprintf(xh_txt, "%lld %d\n", rr_intervals->time[i], xh[i]);
                fflush(xh_txt);
                fprintf(wv_txt, "%lld %lld\n", rr_intervals->time[i], wv[i]); // Changed from wv_u32[i] to wv[i]
                fflush(wv_txt);
                fprintf(sy_txt, "%lld %d\n", rr_intervals->time[i], sy[i]);
                fflush(sy_txt);
                fprintf(entropy_txt, "%lld %f\n", rr_intervals->time[i], entropy_f32[i]);
                fflush(entropy_txt);
            }

#if GNUPLOT_VISUALIZATION == GNUPLOT_VISUALIZATION_ENABLED
            GNUpipe = popen("gnuplot -persist", "w");
            if (GNUpipe == NULL)
            {
                printf("Failed to open GNUpipe\n");
            }

            fprintf(GNUpipe, "set term qt persist\n");
            fprintf(GNUpipe, "set title 'Signal Analysis'\n");
            // Plot each file in its own Gnuplot terminal
            if (GNUpipe)
            {
                // Set high-resolution terminal with larger window size
                fprintf(GNUpipe, "set terminal qt size 3840,2160 enhanced font 'Arial,12' persist\n");
                fprintf(GNUpipe, "set output\n");

                // Set up multiplot layout (3x3 grid)
                fprintf(GNUpipe, "set multiplot layout 3,3 title 'Signal Processing Pipeline Analysis'\n");
                fprintf(GNUpipe, "set grid\n");
                fprintf(GNUpipe, "unset key\n");

                // Plot 1: Original RR uint32_tervals (x)
                fprintf(GNUpipe, "set title 'Original RR Intervals (x)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'RR Interval (ms)'\n");
                fprintf(GNUpipe, "plot 'gen/x_txt' with lines linewidth 2 linecolor rgb 'blue'\n");

                // Plot 2: Median filtered signal (y)
                fprintf(GNUpipe, "set title 'Median Filtered Signal (y)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'Filtered RR (ms)'\n");
                fprintf(GNUpipe, "plot 'gen/y_txt' with lines linewidth 2 linecolor rgb 'red' \n");

                // Plot 3: Low-reference filtered signal (xl)
                fprintf(GNUpipe, "set title 'Low-reference Filtered Signal (xl)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'Low-reference Output'\n");
                fprintf(GNUpipe, "plot 'gen/xl_txt' with lines linewidth 2 linecolor rgb 'green', "
                                 "'gen/xh_txt' with lines linewidth 2 linecolor rgb 'purple'\n");

                // Plot 4: High-reference filtered signal (xh)
                // fprintf(GNUpipe, "set title 'High-reference Filtered Signal (xh)'\n");
                // fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                // fprintf(GNUpipe, "set ylabel 'High-reference Output'\n");
                // fprintf(GNUpipe, "plot 'xh_txt' with lines linewidth 2 linecolor rgb 'purple'\n");

                // Plot 5: Delta_RR signal (delta_x)
                fprintf(GNUpipe, "set title 'Delta_RR Signal (delta_x)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'Delta_RR Output'\n");
                fprintf(GNUpipe, "plot 'gen/delta_x_txt' with lines linewidth 2 linecolor rgb 'orange'\n");

                // Plot 6: Symbolic dynamic signal (sy)
                fprintf(GNUpipe, "set title 'Symbol Sequence (sy)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'Symbolic Sequence'\n");
                fprintf(GNUpipe, "plot 'gen/sy_txt' with lines linewidth 2 linecolor rgb 'purple'\n");

                // Plot 7: Word sequence signal (wv)
                fprintf(GNUpipe, "set title 'Word Sequence (wv)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'Word Sequence'\n");
                fprintf(GNUpipe, "plot 'gen/wv_txt' with lines linewidth 2 linecolor rgb 'purple'\n");

                // Plot 8: Shannon entropy signal (se)
                fprintf(GNUpipe, "set title 'Shannon Entropy (se)'\n");
                fprintf(GNUpipe, "set xlabel 'Sample Index'\n");
                fprintf(GNUpipe, "set ylabel 'Shannon Entropy'\n");
                fprintf(GNUpipe, "plot 'gen/entropy_txt' with lines linewidth 2 linecolor rgb 'purple', "
                                 "0.353 with lines linewidth 2 linecolor rgb 'red' \n");

                // Plot 9: Entropy histogram (variable-width boxes)
                fprintf(GNUpipe, "set title 'Entropy Histogram'\n");
                fprintf(GNUpipe, "set xlabel 'Entropy'\n");
                fprintf(GNUpipe, "set ylabel 'Count'\n");
                fprintf(GNUpipe, "set style fill solid 0.6 noborder\n");
                fprintf(GNUpipe, "plot 'gen/entropy_hist_txt' using (($1+$2)/2):3:(($2-$1)) with boxes lc rgb 'gray'\n");

                // End multiplot
                fprintf(GNUpipe, "unset multiplot\n");
                fflush(GNUpipe);
            }
            if (GNUpipe)
                pclose(GNUpipe);
#endif

            // Clean up per-record resources (do not call wfdbquit here)
            if (x_txt)
                fclose(x_txt);
            if (y_txt)
                fclose(y_txt);
            if (xl_txt)
                fclose(xl_txt);
            if (xh_txt)
                fclose(xh_txt);
            if (wv_txt)
                fclose(wv_txt);
            if (sy_txt)
                fclose(sy_txt);
            if (seg_afib_note_txt)
                fclose(seg_afib_note_txt);
            if (entropy_txt)
                fclose(entropy_txt);
        }

        // Write average performance metrics across all records
        performance_metrics_write_averages(all_metrics, record_size_u8);
#endif
        // Clean up global resources after all records are processed
#if (BUILDCONFIG == BUILDCONFIG_DEBUG)
        if (all_metrics)
            free(all_metrics);
#endif
        rr_intervals_free(rr_intervals);
        if (actual_u8)
            free(actual_u8);
    }
    wfdbquit();
    exit(0);
}
