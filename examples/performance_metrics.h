#ifndef PERFORMANCE_METRICS_H
#define PERFORMANCE_METRICS_H

#include "project.h"

/**
 * Initialize performance metrics structure
 * @param metrics: Pointer to performance metrics structure
 */
void performance_metrics_init(performance_metrics_t *metrics);

/**
 * Update confusion matrix with new predictions
 * @param metrics: Pointer to performance metrics structure
 * @param actual: Array of actual labels (0 or 1)
 * @param predicted: Array of predicted labels (0 or 1)
 * @param length: Length of arrays
 */
void performance_metrics_update(performance_metrics_t *metrics, const uint8_t *actual, const int8_t *predicted, size_t length);

/**
 * Calculate performance metrics from confusion matrix
 * @param metrics: Pointer to performance metrics structure
 */
void performance_metrics_calculate(performance_metrics_t *metrics);

/**
 * Print performance metrics to console
 * @param metrics: Pointer to performance metrics structure
 * @param record_name: Name of the record
 */
void performance_metrics_print(const performance_metrics_t *metrics, const char *record_name);

/**
 * Write performance metrics to file
 * @param metrics: Pointer to performance metrics structure
 * @param record_name: Name of the record
 */
void performance_metrics_write_to_file(const performance_metrics_t *metrics, const char *record_name);

/**
 * Reset/clear the results file at the beginning of a new project run
 */
void performance_metrics_reset_file(void);

/**
 * Write average performance metrics across all records to file
 * @param total_metrics: Array of performance metrics for all records
 * @param num_records: Number of records processed
 */
void performance_metrics_write_averages(const performance_metrics_t *total_metrics, size_t num_records);

/**
 * Export performance metrics to CSV file
 * @param metrics: Pointer to performance metrics structure
 * @param record_name: Name of the record
 */
void export_metrics_to_csv(const performance_metrics_t *metrics, const char *record_name);

/**
 * Export actual vs predicted labels to text file for comparison
 * @param actual: Array of actual labels (0 or 1)
 * @param predicted: Array of predicted labels (0 or 1) 
 * @param length: Length of arrays
 * @param record_name: Name of the record
 */
void export_comparison_to_file(const uint8_t *actual, const int8_t *predicted, const char *record_name, rr_intervals_t *rr_intervals);

#endif /* PERFORMANCE_METRICS_H */
