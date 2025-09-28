// In word_sequence.c
#include "word_sequence.h"
#include <stdio.h>

/**
 * Initialize the word sequence filter state.
 * @param state: Pointer to the filter's state structure.
 */
void word_sequence_init(word_sequence_state_t *state) {
	if (!state) {
		return;
	}
	state->s1 = 0;
	state->s2 = 0;
}

/**
 * Processes a single symbol to produce a word sequence value.
 * @param state: Pointer to the filter's state structure.
 * @param current_sy: The current input symbol (s0).
 * @return: The calculated word value.
 */
int32_t word_sequence_update(word_sequence_state_t *state, uint8_t current_sy) {
	if (!state) {
		return 0; // Return a default value on error
	}

	// Ensure the current symbol is within the valid range [0-9]
	uint8_t s0 = (current_sy <= 9) ? current_sy : 0;

	// Retrieve the previous two symbols from the state
	uint8_t s1 = state->s1;
	uint8_t s2 = state->s2;

	// Create the word value using the current and previous symbols
	int32_t word_value = (s2 << 8) | (s1 << 4) | s0;

	// Update the state for the next function call
	// The current s1 becomes the next s2
	state->s2 = s1;
	// The current s0 becomes the next s1
	state->s1 = s0;

	// Clamp the value if it exceeds the known buffer limit from the
	// shannon_entropy function
	if (word_value >= 2458) {
		fprintf(stderr,
				"word_sequence: Warning - word value %ld exceeds buffer "
				"limit, clamping to 2457\n",
				word_value);
		word_value = 2457;
	}

	return word_value;
}