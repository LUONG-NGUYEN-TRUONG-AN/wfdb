/*
 * symbolic_dynamic.c
 *
 *  Created on: Oct 14, 2025
 *      Author: luong
 */

// In symbolic_dynamic.c
#include "symbolic_dynamic.h"
#include <string.h>

/**
 * Initialize the symbolic dynamic filter state
 */
void symbolic_dynamic_init(symbolic_dynamic_state_t *state) {
	if (!state)
		return;
	memset(state->x_buffer, 0, sizeof(state->x_buffer));
	memset(state->xl_buffer, 0, sizeof(state->xl_buffer));
	state->x_index = 0;
	state->xl_index = 0;
}

/**
 * Processes a single sample to produce a symbolic output.
 * @param state: Pointer to the filter's state structure.
 * @param x: The current raw RR-interval sample.
 * @param xl: The current low-pass filtered sample.
 * @param xh: The current high-pass filtered sample.
 * @return: The symbolic output (0-9).
 */
uint8_t symbolic_dynamic_update(symbolic_dynamic_state_t *state, uint16_t x,
								uint16_t xl, uint16_t xh) {
	if (!state)
		return 0; // Return a default symbol on error

	// --- Step 1: Calculate DeltaRR using historical data ---

	// Get the delayed samples from the circular buffers
	uint16_t x_delayed = state->x_buffer[state->x_index];
	uint16_t xl_delayed = state->xl_buffer[state->xl_index];

	// Store the new samples, overwriting the oldest ones
	state->x_buffer[state->x_index] = x;
	state->xl_buffer[state->xl_index] = xl;

	// Advance the circular buffer indices
	state->x_index = (state->x_index + 1) % SYMBOLIC_X_DELAY;
	state->xl_index = (state->xl_index + 1) % SYMBOLIC_XL_DELAY;

	// Calculate the delta for the current step
	int16_t delta_rr = x_delayed - xl_delayed;

	// --- Step 2: Calculate thresholds and determine the symbol ---

	uint16_t thres1 = xh / 16; // xh / 16
	uint16_t thres2 = xh / 8;  // xh / 8
	uint16_t thres3 = thres1 + thres2;
	uint16_t thres4 = xh / 4; // xh / 4
	uint16_t thres5 = thres4 + thres1;

	uint8_t sy;
	if (delta_rr < -thres4)
		sy = 0;
	else if (delta_rr < -thres3)
		sy = 1;
	else if (delta_rr < -thres2)
		sy = 2;
	else if (delta_rr < -thres1)
		sy = 3;
	else if (delta_rr < thres1)
		sy = 4;
	else if (delta_rr < thres2)
		sy = 5;
	else if (delta_rr < thres3)
		sy = 6;
	else if (delta_rr < thres4)
		sy = 7;
	else if (delta_rr < thres5)
		sy = 8;
	else
		sy = 9;

	return sy;
}
