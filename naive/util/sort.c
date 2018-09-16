//
//  sort.c
//  math
//
//  Created by Alexandr Kondratyev on 02/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "sort.h"

void merge(float *Values, size_t front, size_t middle, size_t back);

/* Uniq */
float *uniq_floats(float *values, size_t size, size_t *new_size_ptr) {
    float *uniq = malloc(size * sizeof(float));
    
    size_t front, back;
    size_t new_size = 1;
    
    uniq[0] = values[0];
    
    for (front = 1; front < size; front++)
    {
        for (back = 0; back < new_size; back++) {
            if(values[front] == uniq[back]) {
                break;
            }
        }
        
        if(back == new_size) {
            uniq[new_size++] = values[front];
        }
    }
    
    uniq = realloc(uniq, new_size * sizeof(float));
    *new_size_ptr = new_size;
    
    return uniq;
}

char **uniq_strings(char **values, size_t size, size_t *new_size_ptr) {
    char **uniq = malloc(sizeof(char*));
    
    size_t front, back;
    size_t new_size = 1;
    
    uniq[0] = values[0];
    
    for (front = 1; front < size; front++)
    {
        uniq = realloc(uniq, (new_size + 1) * sizeof(char*));
        
        for (back = 0; back < new_size; back++) {
            if(strcmp(values[front], uniq[back]) == 0) {
                break;
            }
        }
        
        if(back == new_size) {
            uniq[new_size++] = values[front];
        }
    }
    
    uniq[new_size++] = NULL;
    *new_size_ptr = new_size;
    
    return uniq;
}

/* Sort */
void merge_sort(float *Values, size_t front, size_t back) {
    if(front >= back) {
        return;
    }
    
    size_t middle = (front + back) / 2;
    
    merge_sort(Values, front, middle);
    merge_sort(Values, middle + 1, back);
    
    merge(Values, front, middle, back);
}

void merge(float *Values, size_t front, size_t middle, size_t back) {
    size_t left_size = middle - front + 1;
    size_t right_size = back - middle;
    
    float Left[left_size], Right[right_size];
    for (size_t index = 0; index < left_size; index++){
        Left[index] = *(Values + front + index);
    }
    for (size_t index = 0; index < right_size; index++) {
        Right[index] = *(Values + middle + index + 1);
    }
    
    size_t left_pos = 0;
    size_t right_pos = 0;
    size_t values_pos = front;
    
    while(left_pos < left_size && right_pos < right_size) {
        if(Left[left_pos] <= Right[right_pos]) {
            Values[values_pos] = Left[left_pos];
            left_pos++;
        } else {
            Values[values_pos] = Right[right_pos];
            right_pos++;
        }
        values_pos++;
    }
    
    while(left_pos < left_size) {
        Values[values_pos] = Left[left_pos];
        left_pos++;
        values_pos++;
    }
    
    while(right_pos < right_size) {
        Values[values_pos] = Right[right_pos];
        right_pos++;
        values_pos++;
    }
}

