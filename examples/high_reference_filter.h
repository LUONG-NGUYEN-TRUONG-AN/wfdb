#ifndef HIGH_REFERENCE_FILTER_H
#define HIGH_REFERENCE_FILTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HIGH_REFERENCE_FILTER_N       32u                      /* comb length        */
#define HIGH_REFERENCE_FILTER_MASK    (HIGH_REFERENCE_FILTER_N - 1u)  /* 0x1F       */
#define HIGH_REFERENCE_FILTER_SHIFT   11u                      /* log2(N^2) = log2(2048) */

typedef struct {
    uint16_t buffer_32[HIGH_REFERENCE_FILTER_N];
    uint16_t buffer_64[HIGH_REFERENCE_FILTER_N];
    uint16_t buffer_96[HIGH_REFERENCE_FILTER_N]; 
    int32_t  y[2];   
    uint8_t  idx;     
} RCfilter_high_t;

void high_reference_filter_init(RCfilter_high_t *filt);

uint16_t high_reference_filter_update(RCfilter_high_t *filt, uint16_t input);

#ifdef __cplusplus
}
#endif

#endif /* HIGH_REFERENCE_FILTER_H */
