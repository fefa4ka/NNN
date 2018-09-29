//
//  vector.h/Users/fefa4ka/Development/naive/naive/math/vector.h
//  math
//
//  Created by Alexandr Kondratyev on 01/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef vector_h
#define vector_h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "../util/macros.h"
#include "../util/sort.h"

#define NUMBER_TYPE "t_Num"
#define VECTOR_TYPE "t_Vec"
#define VECTOR_HASH_TYPE "t_VectorHash"

#define VECTOR_CHECK_PRINT(vector, message, ...) { check_memory(vector); check((vector)->type == VECTOR_TYPE, "Wrong vector type. " message, ##__VA_ARGS__); check((vector)->size, "Vector size doesn't set. " message, ##__VA_ARGS__); }
#define VECTOR_CHECK(vector) VECTOR_CHECK_PRINT(vector, "")

#define VECTOR(vector, index) *((vector)->values + index)

#define VECTOR_FOREACH(vector) \
    for(size_t index = 0; index < (vector)->size; index++)

#define PI 3.14159265358979323846

typedef struct
{
    char   *type;

    float value;
} number;

typedef struct
{
    char   *type;
    
    size_t size;
    float *values;
} vector;

typedef struct
{
    char *type;
    
    char **keys;
    size_t size;
    vector *index;
} vector_hash;

enum bool {
    false = 0,
    true = 1
    };
    
    struct vector_library_operation {
        vector *    (*vec)(vector *v, vector *w);
        vector *    (*num)(vector *v, float scalar);
    };
    
    struct vector_library {
        vector *  (*create)(size_t size);
        vector *  (*copy)(vector *original);
        vector *  (*reshape)(vector *v, size_t size);
        vector *  (*seed)(vector *instance, float default_value);
        void      (*delete)(void *v);
        
        void      (*print)(vector *v);
        
        struct {
            number *      (*number)(float value);
            vector *      (*floats)(size_t size, float *values);
            vector *      (*strings)(size_t size, char **values);
            
            vector_hash * (*hash)(size_t size, char **values);
        } from;
        
        struct {
            int        (*index_of)(vector *v, float needle);
            vector *   (*unit)(vector *v);
            vector *   (*uniq)(vector *v);
            float      (*length)(vector *v);
            float      (*l_norm)(vector *v, int p);
            float      (*max_norm)(vector *v);
        } prop;
        
        struct {
            float     (*all)(vector *v);
            float     (*to)(vector *v, size_t to_index);
            float     (*between)(vector *v, size_t from_index, size_t to_index);
        } sum;
        
        struct {
            float     (*angle)(vector *v, vector *w);
            enum bool (*is_equal)(vector *v, vector *w);
            enum bool (*is_perpendicular)(vector *v, vector *w);
        } rel;
        
        // Operations
        vector *      (*add)(vector *v, void *term);
        vector *      (*sub)(vector *v, void *subtrahend);
        vector *      (*mul)(vector *v, void *factor);
        vector *      (*div)(vector *v, void *divider);
        
        struct {
            vector *      (*add)(vector *v, float term);
            vector *      (*sub)(vector *v, float subtrahend);
            vector *      (*mul)(vector *v, float factor);
            vector *      (*div)(vector *v, float divider);
        } num;
        
        float       (*dot)(vector *v, vector *w);
        vector *    (*map)(vector *v, double operation(double));
        vector *    (*map_of)(vector *v, float operation(float, float*), float *operation_params);
    };
    
    extern const struct vector_library Vector;
    
    vector *vector_create_ui(void);
    
#endif /* vector_h */
    

