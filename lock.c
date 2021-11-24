#include <stdlib.h>
#include <stdio.h>
#include "lock.h"
#include "paddedprim.h"
#include "unistd.h"
#include <stdatomic.h>

//////////// tas lock/////////////////////////////////////////
void init_lock_tas(struct tas_lock * l){
    atomic_flag_clear(&(l->flag));
}

void lock_tas(struct tas_lock *l){

    while(atomic_flag_test_and_set(&(l->flag))){
        // spinloop
    };
}

void unlock_tas(struct tas_lock *l){
    atomic_flag_clear(&(l->flag));
}



//////////// backoff lock///////////////////////////////////////
void init_lock_backoff(struct backoff_lock * l, int min, int max){
    atomic_flag_clear(&(l->flag));
    l->min = min;
    l->max = max;
}

void lock_backoff(struct backoff_lock *l){

    int backoff = l->min;
    while(1){
       if(!atomic_flag_test_and_set(&(l->flag))){
           return;
       }

        if(backoff > l->max){
            backoff = l->max;
        }
        usleep(backoff);
        int backoff = backoff * 2;
    }
}

void unlock_backoff(struct backoff_lock *l){
    atomic_flag_clear(&(l->flag));
}


////////////array lock//////////////////////////////////////////////

void init_lock_array(struct array_lock * l, int capacity){

    l->capacity = capacity;
    l->arr = malloc(sizeof(PaddedPrimBool_t) * capacity);
    for(int i = 0; i < capacity; i++){
        l->arr[i] = malloc(sizeof(PaddedPrimBool_t));
        l->arr[i]->value = false;
    }
    l->arr[0]->value = true;
    l->tail = 0;
}

void lock_array(struct array_lock * l, int * slot){
    *slot = atomic_fetch_add(&(l->tail), 1) % l->capacity;
    while(! l->arr[*slot]->value){
        // spinloop
    };
}

void unlock_array(struct array_lock * l, int slot){
    l->arr[slot]->value = false;
    int next = (slot + 1) % l->capacity;
    l->arr[next]->value = true;
}

/*
void init_lock_array(struct array_lock * l, int capacity){
    l->capacity = capacity;
    l->arr = malloc(sizeof(volatile int) * capacity);
    for(int i = 0; i < capacity; i++){
        l->arr[i] = 0;
    }
    l->arr[0] = 1;
    l->tail = 0;
}

void lock_array(struct array_lock * l, int * slot){
    *slot = __sync_fetch_and_add(&(l->tail), 1) % l->capacity;
    while(! l->arr[*slot]){
        // spinloop
    };
}

void unlock_array(struct array_lock * l, int slot){
    l->arr[slot] = false;
    int next = (slot + 1) % l->capacity;
    l->arr[next]= true;
}
*/