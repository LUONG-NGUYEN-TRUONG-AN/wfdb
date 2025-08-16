#ifndef PROJECT_H
#define PROJECT_H

// Standard includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h> 
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

// WFDB includes
#include <wfdb/wfdb.h>
#include <wfdb/ecgmap.h>

#define MITDB   0
#define AFDB    1
#define LTAFDB  2
#define DELAY   63

#define DATABASE MITDB // Change this to AFDB or LTAFDB as needed

#ifndef DATABASE
    #error "DATABASE must be defined as MITDB, AFDB, or LTAFDB"
#endif

extern int PiMap[127];
                    
// Function declarations for project.c
WFDB_Time icmp(WFDB_Time *x, WFDB_Time *y);
void median_filter(const WFDB_Time *input, double *output, int length, int flen);
void low_reference_filter(const double *input, double *output, int length);
void high_reference_filter(const double *input, double *output, int length);
void symbolic_dynamic(const WFDB_Time *x, const double *xl, const double *xh, int *sy, int len) ;
void word_sequence(const int *sy, int *wv, int len);
void shannon_entropy_algorithm(const int *inputs, double *results, int input_len, int n);

#endif /* PROJECT_H */