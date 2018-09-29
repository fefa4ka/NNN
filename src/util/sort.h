//
//  sort.h
//  naive
//
//  Created by Alexandr Kondratyev on 14/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef sort_h
#define sort_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

float *uniq_floats(float *values, size_t size, size_t *new_size_ptr);
char  **uniq_strings(char **values, size_t size, size_t *new_size_ptr);

void  merge_sort(float *Values, size_t front, size_t back);

#endif /* sort_h */
