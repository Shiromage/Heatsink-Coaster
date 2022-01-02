// heap.h
// Chase Baker 2022

#ifndef _HEAP_H
#define _HEAP_H

#include <Arduino.h>

#define EFFECT_HEAP_SIZE    512

extern uint8_t EffectHeap[EFFECT_HEAP_SIZE];

void * allocate_heap_space(uint16_t size);
uint16_t heap_space_remaining();
void reset_heap();
void fill_heap(uint8_t pattern_byte);

#endif //_HEAP_H
