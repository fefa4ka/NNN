//
//  set.h
//  naive
//
//  Created by Alexandr Kondratyev on 14/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef set_h
#define set_h

#include <stdio.h>
#include "../math/probability.h"

typedef struct {    
    struct {
        matrix *    values;
        char **     fields;
    } data;
    
    struct {
        char **     labels;
        matrix *    values;
    } features;
    
    struct {
        char **     labels;
        matrix *    values;
    } target;
    
    struct {
        float      mean;
        float      std;
    } normalization;
    
} data_set;

typedef struct {
    size_t   size;
    size_t   count;
    data_set *train;
    data_set **mini;
    data_set *validation;
    data_set *test;
} data_batch;

struct data_library {
    data_set      (*matrix)(matrix *features, matrix *target);
    data_set      (*csv)(char *filename, char** fields, char **target);
    data_batch    (*split)(data_set *set, size_t batch_size, size_t train, size_t validation, size_t test);
    void          (*delete)(data_set *data);
    void          (*print)(data_set *data);
    struct {
        matrix *  (*vector_to_binary)(vector *column);
        vector *  (*binary_to_vector)(matrix *binary);
    } convert;
};

extern const struct data_library Data;


#endif /* set_h */
