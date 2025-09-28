/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "project.h"

static const char *TAG = "File System";

uint32_t PiMap[127] = {
	0,	   7874,  13495, 18265, 22483, 26290, 29770, 32977, 35952, 38723, 41313,
	43740, 46019, 48162, 50181, 52083, 53877, 55569, 57165, 58671, 60092, 61431,
	62693, 63880, 64997, 66047, 67031, 67953, 68815, 69618, 70366, 71059, 71700,
	72290, 72830, 73323, 73770, 74171, 74529, 74843, 75116, 75348, 75541, 75695,
	75811, 75890, 75933, 75941, 75914, 75854, 75760, 75633, 75475, 75285, 75065,
	74815, 74535, 74226, 73889, 73523, 73130, 72710, 72263, 71790, 71292, 70767,
	70218, 69645, 69046, 68425, 67779, 67110, 66419, 65704, 64968, 64209, 63429,
	62628, 61805, 60962, 60098, 59213, 58309, 57385, 56441, 55478, 54495, 53494,
	52474, 51436, 50379, 49305, 48212, 47102, 45974, 44829, 43667, 42488, 41292,
	40080, 38851, 37606, 36345, 35068, 33775, 32467, 31143, 29803, 28449, 27079,
	25695, 24296, 22882, 21453, 20011, 18553, 17082, 15597, 14098, 12585, 11059,
	9519,  7965,  6398,	 4818,	3225,  1619};

/**
 * Initialize a new rr_intervals structure with given initial capacity
 * @param initial_capacity: Initial capacity for the arrays
 * @return: Pointer to initialized rr_intervals_t structure, or NULL on failure
 */
rr_intervals_t *rr_intervals_init(size_t initial_capacity) {
	rr_intervals_t *rr_intervals = NULL;

	// Allocate the structure
	SUALLOC(rr_intervals, 1, sizeof(rr_intervals_t));
	if (!rr_intervals) {
		fprintf(stderr, "Error: rr_intervals allocation failed\n");
		return NULL;
	}

	// Initialize fields
	rr_intervals->length = 0;

	// Allocate arrays
	SUALLOC(rr_intervals->x, initial_capacity, sizeof(WFDB_Time));
	SUALLOC(rr_intervals->actual_u8, initial_capacity, sizeof(uint8_t));
	SUALLOC(rr_intervals->predict_u8, initial_capacity, sizeof(uint8_t));

	if (!rr_intervals->x || !rr_intervals->actual_u8 ||
		!rr_intervals->predict_u8) {
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
void rr_intervals_free(rr_intervals_t *rr_intervals) {
	if (rr_intervals) {
		if (rr_intervals->x)
			free(rr_intervals->x);
		if (rr_intervals->actual_u8)
			free(rr_intervals->actual_u8);
		if (rr_intervals->predict_u8)
			free(rr_intervals->predict_u8);
		free(rr_intervals);
	}
}

/**
 * Expand the capacity of rr_intervals arrays and actual_u8 array
 * @param rr_intervals: Pointer to rr_intervals_t structure
 * @param new_capacity: New capacity for the arrays
 * @return: 0 on success, -1 on failure
 */
int rr_intervals_expand(rr_intervals_t *rr_intervals, size_t new_capacity) {
	if (!rr_intervals) {
		return -1;
	}

	// Reallocate arrays
	SREALLOC(rr_intervals->x, new_capacity, sizeof(WFDB_Time));
	SREALLOC(rr_intervals->actual_u8, new_capacity, sizeof(uint8_t));
	SREALLOC(rr_intervals->predict_u8, new_capacity, sizeof(uint8_t));

	if (!rr_intervals->x || !rr_intervals->actual_u8 ||
		!rr_intervals->predict_u8) {
		fprintf(stderr, "Error: rr_intervals reallocation failed\n");
		return -1;
	}

	return 0;
}

void app_main(void) {
	// Get heap information
	size_t total_heap_size = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
	size_t free_heap_size = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
	size_t min_free_heap_size =
		heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);

	printf("Total Heap Size: %d bytes\n", total_heap_size);
	printf("Free Heap Size: %d bytes\n", free_heap_size);
	printf("Minimum Free Heap Ever: %d bytes\n", min_free_heap_size);
	ESP_LOGI(TAG, "Initializing SPIFFS");

	esp_vfs_spiffs_conf_t conf = {.base_path = "/storage",
								  .partition_label = NULL,
								  .max_files = 5,
								  .format_if_mount_failed = true};

	// Use settings defined above to initialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)",
					 esp_err_to_name(ret));
		}
		return;
	}

	size_t total = 0, used = 0;

	ret = esp_spiffs_info(conf.partition_label, &total, &used);
	if (ret != ESP_OK) {
		ESP_LOGE(
			TAG,
			"Failed to get SPIFFS partition information (%s). Formatting...",
			esp_err_to_name(ret));
		esp_spiffs_format(conf.partition_label);
		return;
	} else {
		/* do nothing */
	}

	if (esp_spiffs_mounted(conf.partition_label) == true) {
		ESP_LOGI(TAG, "Ok Mounted");
	} else {
		ESP_LOGI(TAG, "Not Mounted");
		return;
	}
	// Check consistency of reported partition size info.
	if (used > total) {
		ESP_LOGW(TAG, "Number of used bytes cannot be larger than total. "
					  "Performing SPIFFS_check().");
		ret = esp_spiffs_check(conf.partition_label);
		// Could be also used to mend broken files, to clean unreferenced pages,
		// etc. More info at
		// https://github.com/pellepl/spiffs/wiki/FAQ#powerlosses-contd-when-should-i-run-spiffs_check
		if (ret != ESP_OK) {
			ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
			return;
		} else {
			/* do nothing */
		}
	}

#if (DATABASE == MITDB)
	setwfdb("/storage/");
#elif (DATABASE == AFDB)
	setwfdb("https://physionet.org/files/afdb/1.0.0/");
#elif (DATABASE == LTAFDB)
	setwfdb("https://physionet.org/files/ltafdb/1.0.0/");
#endif
	uint8_t record_size_u8 = sizeof(records) / sizeof(records[0]);
	// {'N', 'L', 'R', 'a', 'V', 'F', 'J', 'A', 'S', 'E', 'j', '/', 'Q'};
	const char **record = records;

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

	if (record != NULL) {
#if (DATABASE == MITDB)
		WFDB_Anninfo a;
		WFDB_Annotation annot;

#elif (DATABASE == AFDB) || (DATABASE == LTAFDB)
		WFDB_Anninfo a[2];
		WFDB_Annotation annot[2];
#endif

		WFDB_Time t0, t1;
		int32_t y, xl, xh, wv;
		float se = 0.0;
		uint8_t sy;
		double sps, tps;
		WFDB_Time a0_u32 = 0, a1_u32 = 0, rr = 0;
		unsigned char start_afib = 0;
		result_t metrics;

		median_filter_t median_filter;
		median_filter_init(&median_filter);

		RCfilter_low_t low_ref_filter;
		low_reference_filter_init(&low_ref_filter);

		RCfilter_high_t high_ref_filter;
		high_reference_filter_init(&high_ref_filter);

		symbolic_dynamic_state_t symbolic_dynamic_state;
		symbolic_dynamic_init(&symbolic_dynamic_state);

		word_sequence_state_t word_sequence_state;
		word_sequence_init(&word_sequence_state);

		shannon_entropy_state_t *shannon_entropy_state = shannon_entropy_init();
		if (shannon_entropy_state == NULL) {
			fprintf(stderr, "Failed to initialize Shannon entropy state\n");
			exit(1);
		}

		result_init(&metrics);
		// Array to store all performance metrics for averaging
		result_t *all_metrics = NULL;
		SUALLOC(all_metrics, record_size_u8, sizeof(result_t));

		// Reset results file at the beginning of each project run
		result_reset_file();
		if (!all_metrics) {
			fprintf(stderr, "Error: all_metrics allocation failed\n");
			exit(1);
		}

		size_t rr_intervals_capacity = 1000; // Initial capacity
		rr_intervals_t *rr_intervals = rr_intervals_init(rr_intervals_capacity);
		if (!rr_intervals) {
			exit(1);
		}

		for (WFDB_Time i = 0; i < record_size_u8; i++) {

			if (record == NULL) {
				exit(1);
			}
			// Reset performance metrics for each record
			result_init(&metrics);

			// Reset for each record
			if (rr_intervals) {
				rr_intervals->length = 0;
			} else {
				fprintf(stderr, "Error: rr_intervals is null\n");
				exit(1);
			}
#if (DATABASE == MITDB)
			a.name = "atr";
			a.stat = WFDB_READ;
			if (annopen((char *)records[i], &a, 1) < 0) {
				exit(1);
			}
			if ((sps = sampfreq((char *)records[i])) < 0.)
				(void)setsampfreq(sps = WFDB_DEFFREQ);

			if ((tps = getiaorigfreq(0)) < sps)
				tps = sps;
#if (BUILDCONFIG == BUILDCONFIG_DEBUG)
			double tpm, tph, n;
			int sprec, mprec, hprec;
			sprec = 3;
			n = 1000;
			while (n < tps) {
				sprec++;
				n *= 10;
			}

			tpm = 60.0 * tps;
			mprec = 5;
			n = 100000;
			while (n < tpm) {
				mprec++;
				n *= 10;
			}

			tph = 60.0 * tpm;
			hprec = 7;
			n = 10000000;
			while (n < tph) {
				hprec++;
				n *= 10;
			}
#endif
			a0_u32 = NOTQRS;
			t0 = 0;
			while (getann(0, &annot) == 0) {
				a1_u32 = annot.anntyp;
				if (tps == sps) {
					t1 = annot.time;
				} else {
					t1 = annot.time * sps / tps + 0.5;
					if (t1 > annot.time * sps / tps + 0.5)
						t1--;
				}
				if (annot.anntyp == RHYTHM &&
					strncmp((char *)(annot.aux + 1), "(AFIB", 5) == 0) {
					start_afib = 1;
				} else if (annot.anntyp == RHYTHM &&
						   strncmp((char *)(annot.aux + 1), "(AFIB", 5) != 0) {
					start_afib = 0;
				}

				/* Does t1 mark a valid interval starting point? */
				if (a1_u32 <= ACMAX &&
					((a1f_u8[0] && isqrs(a1_u32)) || a1f_u8[a1_u32])) {
					// Expand capacity if needed
					if (rr_intervals->length >= rr_intervals_capacity) {
						rr_intervals_capacity *= 2;
						if (rr_intervals_expand(rr_intervals,
												rr_intervals_capacity) != 0) {
							rr_intervals_free(rr_intervals);
							exit(1);
						}
					}

					// Store RR interval
					rr = t1 - t0;
					rr_intervals->x[rr_intervals->length] = t1 - t0;

					// Store AF label (ground truth)
					if (start_afib) {
						rr_intervals->actual_u8[rr_intervals->length] = 1;
					} else {
						rr_intervals->actual_u8[rr_intervals->length] = 0;
					}

					// --- Real-time processing to get Shannon Entropy ---
					y = median_filter_update(&median_filter, rr);
					xl = low_reference_filter_update(&low_ref_filter, y);
					xh = high_reference_filter_update(&high_ref_filter, xl);
					sy = symbolic_dynamic_update(&symbolic_dynamic_state, rr,
												 xl, xh);
					wv = word_sequence_update(&word_sequence_state, sy);
					se = shannon_entropy_update(shannon_entropy_state, wv);
					// Make a prediction based on the Shannon Entropy and store
					// it
					rr_intervals->predict_u8[rr_intervals->length] =
						(se >= 0.353) ? 1 : 0;

					rr_intervals->length++;
				}
				/* Does t1 mark a valid interval ending point? */
				if (a1_u32 <= ACMAX &&
					((a0f_u8[0] && isqrs(a1_u32)) || a0f_u8[a1_u32])) {
					a0_u32 = a1_u32;
					t0 = t1;
				}
			}
#elif (DATABASE == AFDB) || (DATABASE == LTAFDB)
			a[0].name = "qrsc";
			a[0].stat = WFDB_READ;
			a[1].name = "atr";
			a[1].stat = WFDB_READ;
			if (annopen((char *)records[i], a, 2) < 0) {
				exit(2);
			}
			if ((sps = sampfreq((char *)records[i])) < 0.)
				(void)setsampfreq(sps = WFDB_DEFFREQ);

			if ((tps = getiaorigfreq(0)) < sps)
				tps = sps;
#if (BUILDCONFIG == BUILDCONFIG_DEBUG)
			double tpm, tph, n;
			int sprec, mprec, hprec;
			sprec = 3;
			n = 1000;
			while (n < tps) {
				sprec++;
				n *= 10;
			}

			tpm = 60.0 * tps;
			mprec = 5;
			n = 100000;
			while (n < tpm) {
				mprec++;
				n *= 10;
			}

			tph = 60.0 * tpm;
			hprec = 7;
			n = 10000000;
			while (n < tph) {
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

			while (qrs_available || rhythm_available) {
				// Process the annotation with the earlier timestamp
				if (rhythm_available &&
					(!qrs_available || next_rhythm.time <= next_qrs.time)) {
					// Process rhythm annotation to update AF state
					if (next_rhythm.anntyp == RHYTHM && next_rhythm.aux &&
						next_rhythm.aux[0] > 0) {
						if (strncmp((char *)(next_rhythm.aux + 1), "(AFIB",
									5) == 0) {
							start_afib = 1;
						} else {
							start_afib = 0;
						}
					}
					rhythm_available = (getann(1, &next_rhythm) == 0);
				} else if (qrs_available) {
					// Process QRS annotation using current AF state
					annot[0] = next_qrs;
					a1_u32 = annot[0].anntyp;
					if (tps == sps) {
						t1 = annot[0].time;
					} else {
						t1 = annot[0].time * sps / tps + 0.5;
						if (t1 > annot[0].time * sps / tps + 0.5)
							t1--;
					}
					/* Does t1 mark a valid interval starting point? */
					if (a1_u32 <= ACMAX &&
						((a1f_u8[0] && isqrs(a1_u32)) || a1f_u8[a1_u32])) {
						// Expand capacity if needed
						if (rr_intervals->length >= rr_intervals_capacity) {
							rr_intervals_capacity *= 2;
							if (rr_intervals_expand(
									rr_intervals, rr_intervals_capacity) != 0) {
								rr_intervals_free(rr_intervals);
								exit(1);
							}
						}

						// Store RR interval
						rr_intervals->x[rr_intervals->length] = t1 - t0;

						// Store current AF state (ground truth)
						rr_intervals->actual_u8[rr_intervals->length] =
							start_afib;

						rr_intervals->length++;
						// --- Real-time processing to get Shannon Entropy ---
						y = median_filter_update(&median_filter, rr);
						xl = low_reference_filter_update(&low_ref_filter, y);
						xh = high_reference_filter_update(&high_ref_filter, xl);
						sy = symbolic_dynamic_update(&symbolic_dynamic_state, rr, xl, xh);
						wv = word_sequence_update(&word_sequence_state, sy);
						se = shannon_entropy_update(shannon_entropy_state, wv);
						// Make a prediction based on the Shannon Entropy and  store it
						rr_intervals->predict_u8[rr_intervals->length] =
							(se >= 0.353) ? 1 : 0;
					}

					/* Does t1 mark a valid interval ending point? */
					if (a1_u32 <= ACMAX &&
						((a0f_u8[0] && isqrs(a1_u32)) || a0f_u8[a1_u32])) {
						a0_u32 = a1_u32;
						t0 = t1;
					}

					qrs_available = (getann(0, &next_qrs) == 0);
				}
			}
#endif
			// Check if we collected any RR intervals
			if (rr_intervals->length == 0) {
				fprintf(stderr,
						"Warning: No RR intervals found for record %s\n",
						records[i]);
				continue;
			}

			// Update performance metrics with all collected predictions
			result_update(&metrics, rr_intervals->actual_u8,
						  rr_intervals->predict_u8, rr_intervals->length);

			// Calculate final performance metrics from the accumulated counts
			result_calculate(&metrics);

			// Store metrics for this record in the array for averaging
			all_metrics[i] = metrics;

			// Print performance metrics to console
			result_print(&metrics, records[i]);
		}

		// Write average performance metrics across all records
		result_write_averages(all_metrics, record_size_u8);
		// Clean up global resources after all records are processed
		if (all_metrics)
			free(all_metrics);

		rr_intervals_free(rr_intervals);
		shannon_entropy_free(
			shannon_entropy_state); // Free the heap-allocated state

		(void)wfdb_anclose();
		printf("Done processing.\n");
	}

	// All done, unmount partition and disable SPIFFS
	esp_vfs_spiffs_unregister(conf.partition_label);
	ESP_LOGI(TAG, "SPIFFS unmounted");

	for (int i = 10; i >= 0; i--) {
		printf("Restarting in %d seconds...\n", i);
		vTaskDelay(100000 / portTICK_PERIOD_MS);
	}
	printf("Restarting now.\n");
	fflush(stdout);
	esp_restart();
	exit(0);
}
