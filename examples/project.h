#ifndef PROJECT_H
#define PROJECT_H

// Standard includes
#include "ecgcodes.h" // For RHYTHM and other annotation constants
#include "ecgmap.h"	  // For isqrs macro and other mappings
#include "esp_chip_info.h"
#include "esp_err.h"
#include "esp_flash.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "wfdb.h"
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#define MITDB 0
#define AFDB 1
#define LTAFDB 2
#define DELAY 63

#define BUILDCONFIG_DEBUG 0
#define BUILDCONFIG_RELEASE 1

#define BUILDCONFIG BUILDCONFIG_RELEASE

#define GNUPLOT_VISUALIZATION_ENABLED 1
#define GNUPLOT_VISUALIZATION_DISABLED 0
#define GNUPLOT_VISUALIZATION GNUPLOT_VISUALIZATION_DISABLED

#define DATABASE AFDB // Change this to AFDB, LTAFDB or MITDB as needed

#ifndef DATABASE
#error "DATABASE must be defined as MITDB, AFDB, or LTAFDB"
#endif

#if (DATABASE == MITDB)
// Complete MITDB records array from RECORDS file
static const char *records[] = {
	"100", "101", "102", "103", "104", "105", "106", "107", "108", "109",
	"111", "112", "113", "114", "115", "116", "117", "118", "119", "121",
	"122", "123", "124", "200", "201", "202", "203", "205", "207", "208",
	"209", "210", "212", "213", "214", "215", "217", "219", "220", "221",
	"222", "223", "228", "230", "231", "232", "233", "234"};
// const char *records[] = {"201"};
#elif (DATABASE == AFDB)
// const char *records[] = {"00735"};
// const char *records[] = {"04048"};
// const char *records[] = {"04015", "04043", "04048"};
static const char *records[] = {"05091"};
#elif (DATABASE == LTAFDB)
static const char *records[] = {
	"00",  "01",  "03",  "05",  "06",  "07",  "08",  "10",  "100", "101",
	"102", "103", "104", "105", "11",  "110", "111", "112", "113", "114",
	"115", "116", "117", "118", "119", "12",  "120", "121", "122", "13",
	"15",  "16",  "17",  "18",  "19",  "20",  "200", "201", "202", "203",
	"204", "205", "206", "207", "208", "21",  "22",  "23",  "24",  "25",
	"26",  "28",  "30",  "32",  "33",  "34",  "35",  "37",  "38",  "39",
	"42",  "43",  "44",  "45",  "47",  "48",  "49",  "51",  "53",  "54",
	"55",  "56",  "58",  "60",  "62",  "64",  "65",  "68",  "69",  "70",
	"71",  "72",  "74",  "75"};
#endif

extern uint32_t PiMap[127];

// Performance metrics structure for classification evaluation
typedef struct performance_metrics {
	WFDB_Time TP;  // True Positive
	WFDB_Time FP;  // False Positive
	WFDB_Time FN;  // False Negative
	WFDB_Time TN;  // True Negative
	WFDB_Time Se;  // Sensitivity
	WFDB_Time Sp;  // Specificity
	WFDB_Time PPV; // Positive Predictive Value
	WFDB_Time ACC; // Accuracy
} result_t;

typedef struct rr_intervals
{
    size_t length;
    WFDB_Time *x;
    uint8_t *actual_u8; // Ground truth label (1 for AF, 0 for normal)
	uint8_t *predict_u8;	  // Prediction label (1 for AF, 0 for normal)
} rr_intervals_t;

// Helper functions for rr_intervals
rr_intervals_t *rr_intervals_init(size_t initial_capacity);
void rr_intervals_free(rr_intervals_t *rr_intervals);
int rr_intervals_expand(rr_intervals_t *rr_intervals, size_t new_capacity);

// Predefined annotation filter arrays
extern const WFDB_Time a0f[ACMAX + 1]; /* annotation filter array for
										  uint32_interval start points */
extern const WFDB_Time
	a1f[ACMAX + 1]; /* annotation filter array for uint32_interval end points */

// Include module headers
#include "high_reference_filter.h"
#include "low_reference_filter.h"
#include "median_filter.h"
#include "performance_metrics.h"
#include "shannon_entropy.h"
#include "symbolic_dynamic.h"
#include "word_sequence.h"

#endif /* PROJECT_H */