#ifndef SYMBOLIC_DYNAMIC_H
#define SYMBOLIC_DYNAMIC_H

#include "project.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "project.h" // For WFDB_Time

#define SYMBOLIC_X_DELAY 63
#define SYMBOLIC_XL_DELAY 47

    // State structure to hold the history for the filter
    typedef struct
    {
        uint16_t x_buffer[SYMBOLIC_X_DELAY];   // Circular buffer for x samples
        uint16_t xl_buffer[SYMBOLIC_XL_DELAY]; // Circular buffer for xl samples
        int x_index;
        int xl_index;
    } symbolic_dynamic_state_t;

    void symbolic_dynamic_init(symbolic_dynamic_state_t *state);
    uint8_t symbolic_dynamic_update(symbolic_dynamic_state_t *state, uint16_t x,
                                    uint16_t xl, uint16_t xh);

#ifdef __cplusplus
}
#endif

#endif /* SYMBOLIC_DYNAMIC_H */
