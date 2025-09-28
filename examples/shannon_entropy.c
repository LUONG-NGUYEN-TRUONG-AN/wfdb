// In shannon_entropy.c
#include "shannon_entropy.h"
#include <string.h>
#include <stdlib.h>

/**
 * Initializes the Shannon entropy calculator state on the heap.
 * @return: Pointer to the allocated state, or NULL on failure.
 */
shannon_entropy_state_t* shannon_entropy_init(void) {
    shannon_entropy_state_t *state = malloc(sizeof(shannon_entropy_state_t));
    if (!state) {
        fprintf(stderr, "shannon_entropy_init: Failed to allocate state\n");
        return NULL;
    }

    memset(state->freq, 0, sizeof(state->freq));
    memset(state->window, 0, sizeof(state->window));
    state->k = 0;
    state->S = 0.0f;
    state->pointer = 0;
    state->count = 0;

    return state;
}

/**
 * Frees the state structure.
 */
void shannon_entropy_free(shannon_entropy_state_t *state) {
    if (state) {
        free(state);
    }
}

/**
 * Processes a single sample and updates the Shannon entropy.
 * @param state: Pointer to the filter's state.
 * @param input: The current input sample (a word value).
 * @return: The calculated Shannon entropy for the current window.
 */
float shannon_entropy_update(shannon_entropy_state_t *state, WFDB_Time input) {
    if (!state) {
        return 0.0f;
    }

    // Clamp input to valid range
    uint32_t idx = (input < SHANNON_MAX_BUFFER) ? input : SHANNON_MAX_BUFFER - 1;

    if (state->count < SHANNON_WINDOW_SIZE) {
        // --- Initialization Phase (filling the first window) ---
        state->window[state->pointer] = idx;
        uint32_t te_in = state->freq[idx];

        if (te_in == 0) state->k++;
        state->freq[idx]++;

        state->S += ((float)PiMap[state->freq[idx]] - (float)PiMap[te_in]);
        state->count++;
    } else {
        // --- Sliding Window Phase ---
        uint32_t old_idx = state->window[state->pointer];
        uint32_t te_in = state->freq[idx];
        uint32_t te_out = state->freq[old_idx];

        state->window[state->pointer] = idx;

        if (state->freq[old_idx] > 0) state->freq[old_idx]--;
        state->freq[idx]++;

        state->S += (((float)PiMap[state->freq[idx]] - (float)PiMap[te_in]) +
                     ((float)PiMap[state->freq[old_idx]] - (float)PiMap[te_out]));

        if (te_in == 0) {
            if (te_out > 1 || te_out == 0) state->k++;
        } else {
            if (state->freq[old_idx] == 0 && te_out == 1) state->k--;
        }
    }

    // Advance the circular buffer pointer
    state->pointer = (state->pointer + 1) % SHANNON_WINDOW_SIZE;

    // Calculate and return the final entropy value for this step
    return (float)((float)(state->k) / SHANNON_WINDOW_SIZE) * (state->S / 1000000.0f);
}