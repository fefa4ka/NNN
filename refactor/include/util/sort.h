#pragma once

#include <stdio.h>

float *uniq_floats(float *values, size_t size, size_t *new_size_ptr);
char  **uniq_strings(char **values, size_t size, size_t *new_size_ptr);

void  merge_sort(float *Values, size_t front, size_t back);

