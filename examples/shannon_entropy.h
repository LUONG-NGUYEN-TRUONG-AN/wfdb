#ifndef SHANNON_ENTROPY_H
#define SHANNON_ENTROPY_H

#include "project.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHANNON_MAX_BUFFER 2458
#define SHANNON_WINDOW_SIZE 127

// State structure to hold all data needed for the algorithm
typedef struct {
    uint32_t freq[SHANNON_MAX_BUFFER];
    uint32_t window[SHANNON_WINDOW_SIZE];
    uint32_t k;       // Cardinality
    float S;          // Shannon entropy sum
    uint32_t pointer; // Circular buffer pointer
    uint32_t count;   // Number of samples processed so far
} shannon_entropy_state_t;

shannon_entropy_state_t* shannon_entropy_init(void);
float shannon_entropy_update(shannon_entropy_state_t *state, uint16_t input);
void shannon_entropy_free(shannon_entropy_state_t *state);

#ifdef __cplusplus
}
#endif


#endif /* SHANNON_ENTROPY_H */
