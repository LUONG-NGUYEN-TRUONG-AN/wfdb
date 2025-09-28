#include "performance_metrics.h"
#include "symbolic_dynamic.h" // For ensure_gen_directory

/**
 * Initialize performance metrics structure
 * @param metrics: Pointer to performance metrics structure
 */
void result_init(result_t *metrics) {
	if (!metrics) {
		fprintf(stderr, "result_init: Invalid input parameter\n");
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
void result_update(result_t *metrics,
								const uint8_t *actual, const uint8_t *predicted,
								size_t length) {
	if (!metrics || !actual || !predicted || length == 0) {
		fprintf(stderr,
				"result_update: Invalid input parameters\n");
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
void result_calculate(result_t *metrics) {
	if (!metrics) {
		fprintf(stderr,
				"result_calculate: Invalid input parameter\n");
		return;
	}

	// Calculate performance metrics with division by zero protection
	metrics->Se = (metrics->TP + metrics->FN > 0)
					  ? (metrics->TP * 100) / (metrics->TP + metrics->FN)
					  : 0;
	metrics->Sp = (metrics->TN + metrics->FP > 0)
					  ? (metrics->TN * 100) / (metrics->TN + metrics->FP)
					  : 0;
	metrics->PPV = (metrics->TP + metrics->FP > 0)
					   ? (metrics->TP * 100) / (metrics->TP + metrics->FP)
					   : 0;
	metrics->ACC =
		(metrics->TP + metrics->TN + metrics->FP + metrics->FN > 0)
			? ((metrics->TP + metrics->TN) * 100) /
				  (metrics->TP + metrics->TN + metrics->FP + metrics->FN)
			: 0;
}

/**
 * Print performance metrics to console
 * @param metrics: Pointer to performance metrics structure
 * @param record_name: Name of the record
 */
void result_print(const result_t *metrics,
							   const char *record_name) {
	if (!metrics || !record_name) {
		fprintf(stderr,
				"result_print: Invalid input parameters\n");
		return;
	}
	printf("Record: %s TP: %ld, FP: %ld, FN: %ld, TN: %ld\n", record_name,
		   metrics->TP, metrics->FP, metrics->FN, metrics->TN);
	printf("Sensitivity (Se): %ld%%, Specificity (Sp): %ld%%, PPV: %ld%%, "
		   "Accuracy: %ld%%\n",
		   metrics->Se, metrics->Sp, metrics->PPV, metrics->ACC);
}

/**
 * Write performance metrics to file
 * @param metrics: Pointer to performance metrics structure
 * @param record_name: Name of the record
 */
void result_write_to_file(const result_t *metrics,
									   const char *record_name) {
	if (!metrics || !record_name) {
		fprintf(
			stderr,
			"result_write_to_file: Invalid input parameters\n");
		return;
	}

	printf("Record: %s\n", record_name);
	printf("TP: %ld, FP: %ld, FN: %ld, TN: %ld\n", metrics->TP, metrics->FP,
		   metrics->FN, metrics->TN);
	printf("Sensitivity (Se): %ld%%, Specificity (Sp): %ld%%, PPV: %ld%%, "
		   "Accuracy: %ld%%\n",
		   metrics->Se, metrics->Sp, metrics->PPV, metrics->ACC);
	printf("-------------------------------------------\n");
}

/**
 * Reset/clear the results file at the beginning of a new project run
 */
void result_reset_file(void) {
	// Reset the results file
}

/**
 * Write average performance metrics across all records to file
 * @param total_metrics: Array of performance metrics for all records
 * @param num_records: Number of records processed
 */
void result_write_averages(
	const result_t *total_metrics, size_t num_records) {
	if (!total_metrics || num_records == 0) {
		fprintf(
			stderr,
			"result_write_averages: Invalid input parameters\n");
		return;
	}

	// Calculate averages
	WFDB_Time avg_Se = 0, avg_Sp = 0, avg_PPV = 0, avg_ACC = 0;
	WFDB_Time valid_Se = 0, valid_Sp = 0, valid_PPV = 0, valid_ACC = 0;

	for (size_t i = 0; i < num_records; i++) {
		// Only include records that have valid metrics (avoid division by zero
		// cases)
		if (total_metrics[i].Se > 0 ||
			(total_metrics[i].TP + total_metrics[i].FN > 0)) {
			avg_Se += total_metrics[i].Se;
			valid_Se++;
		}
		if (total_metrics[i].Sp > 0 ||
			(total_metrics[i].TN + total_metrics[i].FP > 0)) {
			avg_Sp += total_metrics[i].Sp;
			valid_Sp++;
		}
		if (total_metrics[i].PPV > 0 ||
			(total_metrics[i].TP + total_metrics[i].FP > 0)) {
			avg_PPV += total_metrics[i].PPV;
			valid_PPV++;
		}
		if (total_metrics[i].ACC > 0 ||
			(total_metrics[i].TP + total_metrics[i].TN + total_metrics[i].FP +
				 total_metrics[i].FN >
			 0)) {
			avg_ACC += total_metrics[i].ACC;
			valid_ACC++;
		}
	}

	// Calculate final averages
	avg_Se = (valid_Se > 0) ? avg_Se / valid_Se : 0;
	avg_Sp = (valid_Sp > 0) ? avg_Sp / valid_Sp : 0;
	avg_PPV = (valid_PPV > 0) ? avg_PPV / valid_PPV : 0;
	avg_ACC = (valid_ACC > 0) ? avg_ACC / valid_ACC : 0;

	// Also print to console
	printf("Average Sensitivity (Se): %ld%%\n", avg_Se);
	printf("Average Specificity (Sp): %ld%%\n", avg_Sp);
	printf("Average PPV: %ld%%\n", avg_PPV);
	printf("Average Accuracy: %ld%%\n", avg_ACC);
	printf("=====================================\n");
}

/**
 * Export performance metrics to CSV file
 * @param metrics: Pointer to performance metrics structure
 * @param record_name: Name of the record
 */
void export_metrics_to_csv(const result_t *metrics,
						   const char *record_name) {
	if (!metrics || !record_name) {
		fprintf(stderr, "export_metrics_to_csv: Invalid input parameters\n");
		return;
	}

	// NOTE: Header is not printed here to avoid repetition for each record.
	// A calling function could print it once.
	// printf("Record,TP,FP,FN,TN,Se,Sp,PPV,ACC\n");

	// Write metrics data to stdout
	printf("%s,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld\n", record_name, metrics->TP,
		   metrics->FP, metrics->FN, metrics->TN, metrics->Se, metrics->Sp,
		   metrics->PPV, metrics->ACC);

	printf("Metrics exported to stdout\n");
}
