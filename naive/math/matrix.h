//
//  matrix.h
//  math
//
//  Created by Alexandr Kondratyev on 01/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef matrix_h
#define matrix_h

#include <stdio.h>
#include "vector.h"
#include "../data/csv.h"

#define MATRIX(matrix, row, column) *((matrix)->vector->values + row * ((matrix)->columns) + column)
#define MATRIX_TYPE "t_Mat"
#define MATRIX_FOREACH(matrix) \
    for(size_t row = 0; row < matrix->rows; row++) \
        for (size_t column = 0; column < (matrix)->columns ; column++) \

typedef struct
{
    char   *type;
    
    size_t rows;
    size_t columns;
    vector *vector;
} matrix;

struct matrix_library_operation {
    matrix *        (*mat)(matrix *A, matrix *B);
    matrix *        (*vec)(matrix *A, vector *v);
    matrix *        (*num)(matrix *A, float scalar);
};

struct matrix_library {
    matrix *        (*create)(size_t rows, size_t columns);
    matrix *        (*from)(void *data, size_t rows, size_t columns);
    matrix *        (*csv)(csv *file, char **fields);
    matrix *        (*copy)(matrix *original);
    matrix *        (*seed)(matrix *A, float default_value);
    matrix *        (*reshape)(matrix *instance, size_t rows, size_t columns);
    void            (*delete)(matrix *A);
    
    void            (*print)(matrix *A);
    
    matrix *        (*identity)(size_t size);
    matrix *        (*diagonal)(vector *v);
    
    vector *        (*column)(matrix *A, size_t column);
    
    struct {
        float       (*sum)(matrix *A);
        float       (*trace)(matrix *A);
        float       (*frobenius_norm)(matrix *A);
    } prop;
    
    struct {
        enum bool   (*is_equal)(matrix *A, matrix *B);
    } rel;
    
    matrix *        (*add)(matrix *A, void *term);
    matrix *        (*sub)(matrix *A, void *subtrahend);
    matrix *        (*mul)(matrix *A, void *factor);
    matrix *        (*div)(matrix *A, void *divider);
    
    matrix *        (*transpose)(matrix *A);
    matrix *        (*map)(matrix *A, float operation(float));
};

extern const struct matrix_library Matrix;

#endif /* matrix_h */

