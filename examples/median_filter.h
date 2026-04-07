#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MEDIAN_FILTER_SIZE 17

/**
 * Mathematical Formulation:
 * -------------------------
 * For input sequence x_i, define S as a sorted array of successive
 * elements from x_{i-2w-1} to x_{i-1}, where w = floor(W/2) and
 * W = MEDIAN_FILTER_SIZE.
 *
 * For each new sample x_i:
 *
 *   Step 1: Binary search to find position m of departing sample x_{i-2w-1}
 *   Step 2: Binary search to find insertion position t for incoming x_i
 *   Step 3: Shift elements in sorted[] from m to t (maintain order)
 *           - If m < t: shift left (s_r ← s_{r+1} for r ∈ [m, t-1])
 *           - If m > t: shift right (s_r ← s_{r-1} for r ∈ [t+1, m])
 *   Step 4: Insert x_i at sorted[t]
 *   Step 5: Output y_i = S_{w+1} = sorted[W/2] (median)
 *
 * Time Complexity:
 *   - Filling phase: O(W) per update (insertion into unsorted region)
 *   - Sliding phase: O(W) per update (two shifts in worst case)
 * Space Complexity: O(W) static allocation (no dynamic memory)
 * ================================================================
 */

typedef struct
{
	uint16_t window[MEDIAN_FILTER_SIZE];
	uint16_t sorted[MEDIAN_FILTER_SIZE];
	uint16_t head;
	uint16_t count;
} median_filter_t;


void median_filter_init(median_filter_t *filt);

/**
 * Update median filter with a new sample (recursive sliding-window).
 *
 * Implements the recursive median filter algorithm:
 *   1. Binary search for departing sample position (m)
 *   2. Binary search for insertion position (t)
 *   3. Shift elements from m to t
 *   4. Insert new sample at position t
 *   5. Return median at index W/2
 *
 * @param filt:       Pointer to filter state.
 * @param new_sample: Current input x_i.
 * @return:           Median output y_i = sorted[W/2].
 *
 * Non-blocking, deterministic O(W) execution time.
 */
uint16_t median_filter_update(median_filter_t *filt, uint16_t new_sample);

void median_filter_reset(median_filter_t *filt);
uint16_t median_filter_get_count(const median_filter_t *filt);
uint16_t median_filter_get_median(const median_filter_t *filt);

#ifdef __cplusplus
}
#endif

#endif /* MEDIAN_FILTER_H */
