#include "high_reference_filter.h"
#include <string.h>

void high_reference_filter_init(RCfilter_high_t *filt)
{
    if (!filt) return;
    memset(filt->buffer_32, 0, sizeof(filt->buffer_32));
    memset(filt->buffer_64, 0, sizeof(filt->buffer_64));
    memset(filt->buffer_96, 0, sizeof(filt->buffer_96));
    filt->y[0] = 0;
    filt->y[1] = 0;
    filt->idx  = 0;
}

uint16_t high_reference_filter_update(RCfilter_high_t *filt, uint16_t input)
{
    if (!filt) return 0;

    uint8_t idx = filt->idx;

    int32_t x_32 = (int32_t)filt->buffer_32[idx];   
    int32_t x_64 = (int32_t)filt->buffer_64[idx];   
    int32_t x_96 = (int32_t)filt->buffer_96[idx];   

    filt->buffer_96[idx] = (uint16_t)x_64;          
    filt->buffer_64[idx] = (uint16_t)x_32;         
    filt->buffer_32[idx] = input; 

    filt->idx = (uint8_t)((idx + 1u) & HIGH_REFERENCE_FILTER_MASK);

    int32_t y_new = (filt->y[0] << 1)           
                  - filt->y[1]                      
                  + (int32_t)input                   
                  - x_32                            
                  - x_64                            
                  + x_96;                           


    filt->y[1] = filt->y[0];
    filt->y[0] = y_new;

    int32_t scaled = y_new >> HIGH_REFERENCE_FILTER_SHIFT;

    if (scaled < 0)     scaled = 0;
    if (scaled > 65535) scaled = 65535;

    return (uint16_t)scaled;
}
