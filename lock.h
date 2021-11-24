#ifndef LOCK_H
#define LOCK_H

#include "paddedprim.h"
#include <stdatomic.h>

struct tas_lock{
    volatile atomic_flag flag;
};

struct backoff_lock{

    volatile atomic_flag flag;

    // min and max backoff time, set by the initializer (microseconds)
    int min;
    int max;
};


struct array_lock{

    int capacity;
    volatile atomic_int tail;
    PaddedPrimBool_t ** arr;

};


/*
struct array_lock{

    int capacity;
    volatile int tail;
    volatile int *arr;
};
*/

void init_lock_tas(struct tas_lock * l);
void lock_tas(struct tas_lock *l);
void unlock_tas(struct tas_lock *l);
void init_lock_backoff(struct backoff_lock * l, int min, int max);
void lock_backoff(struct backoff_lock *l);
void unlock_backoff(struct backoff_lock *l);
void init_lock_array(struct array_lock * l, int capacity);
void lock_array(struct array_lock * l, int * slot);
void unlock_array(struct array_lock * l, int slot);


#endif