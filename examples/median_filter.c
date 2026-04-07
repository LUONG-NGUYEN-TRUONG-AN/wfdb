#include "median_filter.h"
#include <string.h>

static uint16_t binary_search_exact(const uint16_t a[], uint16_t n, uint16_t key)
{
	uint16_t low = 0;
	uint16_t high = n;

	while (low < high)
	{
		uint16_t mid = low + (high - low) / 2;

		if (a[mid] < key)
		{
			low = mid + 1;
		}
		else if (a[mid] > key)
		{
			high = mid;
		}
		else
		{
			return mid; 
		}
	}
	return n; 
}

static uint16_t lower_bound(const uint16_t a[], uint16_t n, uint16_t key)
{
	uint16_t low = 0;
	uint16_t high = n;

	while (low < high)
	{
		uint16_t mid = low + (high - low) / 2;

		if (a[mid] < key)
		{
			low = mid + 1;
		}
		else
		{
			high = mid;
		}
	}
	return low;
}

void median_filter_init(median_filter_t *filt)
{
	if (!filt)
		return;

	memset(filt->window, 0, sizeof(filt->window));
	memset(filt->sorted, 0, sizeof(filt->sorted));
	filt->head = 0;
	filt->count = 0;
}

void median_filter_reset(median_filter_t *filt)
{
	if (!filt)
		return;

	median_filter_init(filt);
}

uint16_t median_filter_get_count(const median_filter_t *filt)
{
	if (!filt)
		return 0;

	return filt->count;
}

uint16_t median_filter_get_median(const median_filter_t *filt)
{
	if (!filt)
		return 0;

	if (filt->count == 0)
		return 0;

	return filt->sorted[filt->count / 2];
}

uint16_t median_filter_update(median_filter_t *filt, uint16_t new_sample)
{
	if (!filt)
		return 0;

	if (filt->count < MEDIAN_FILTER_SIZE)
	{
		filt->window[filt->head] = new_sample;

		uint16_t t = lower_bound(filt->sorted, filt->count, new_sample);

		for (uint16_t r = filt->count; r > t; r--)
		{
			filt->sorted[r] = filt->sorted[r - 1];
		}

		filt->sorted[t] = new_sample;

		filt->count++;

		if (++filt->head >= MEDIAN_FILTER_SIZE)
		{
			filt->head = 0;
		}

		return filt->sorted[filt->count / 2];
	}

	uint16_t old_sample = filt->window[filt->head];

	if (new_sample == old_sample)
	{
		filt->window[filt->head] = new_sample;

		if (++filt->head >= MEDIAN_FILTER_SIZE)
		{
			filt->head = 0;
		}

		return filt->sorted[MEDIAN_FILTER_SIZE / 2];
	}


	uint16_t m = binary_search_exact(filt->sorted, MEDIAN_FILTER_SIZE, old_sample);

	if (m >= MEDIAN_FILTER_SIZE)
	{
		return filt->sorted[MEDIAN_FILTER_SIZE / 2];
	}

	uint16_t last = MEDIAN_FILTER_SIZE - 1;
	for (uint16_t r = m; r < last; r++)
	{
		filt->sorted[r] = filt->sorted[r + 1];
	}

	uint16_t t = lower_bound(filt->sorted, last, new_sample);

	for (uint16_t r = last; r > t; r--)
	{
		filt->sorted[r] = filt->sorted[r - 1];
	}


	filt->sorted[t] = new_sample;


	filt->window[filt->head] = new_sample;

	if (++filt->head >= MEDIAN_FILTER_SIZE)
	{
		filt->head = 0;
	}
	return filt->sorted[MEDIAN_FILTER_SIZE / 2];
}
