#ifndef __TTABLE_H__
#define __TTABLE_H__

#include <stdlib.h>

// Time table entry
typedef struct {
    clock_t rt;  // release time
    clock_t dl;  // deadline
    clock_t inc; // increment
} ttable_entry_t;

#endif