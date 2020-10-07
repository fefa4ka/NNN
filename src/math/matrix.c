//
//  matrix.c
//  math
//
//  Created by Alexandr Kondratyev on 01/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "matrix.h"


/* Life Cycle */
static
matrix *
matrix_create(size_t rows, size_t columns) {
    check(rows > 0 && columns > 0, "Wrong matrix size");
    matrix *instance = malloc(sizeof(matrix));
    check_memory(instance);
    instance->type = MATRIX_TYPE;
    instance->rows = rows;
    instance->columns = columns;
    instance->vector = Vector.create(rows * columns);
    
    return instance;

error:
    return NULL;
}

static
matrix *
matrix_copy(matrix *original) {
    matrix_check(original);
    matrix *instance = malloc(sizeof(matrix));
    instance->type = MATRIX_TYPE;
    instance->rows = original->rows;
    instance->columns = original->columns;
    instance->vector = Vector.copy(original->vector);
    
    return instance;

error:
    return NULL;
}

static
matrix *
matrix_reshape(matrix *instance, size_t rows, size_t columns) {
    matrix_check(instance);
    check(rows > 0 && columns > 0, "Invalid matrix shape");
    instance->rows = rows;
    instance->columns = columns;
    Vector.reshape(instance->vector, rows * columns);
    
    return instance;

error:
    return NULL;
}

static
void
matrix_delete(matrix *instance) {
    matrix_check(instance);
    Vector.delete(instance->vector);
    free(instance);

error:
    return;
}


/* Data */
static
matrix *
matrix_seed(matrix *instance, float default_value) {
    matrix_check(instance);
    Vector.seed(instance->vector, default_value);
    
    return instance;
error:
    return NULL;
}

static
matrix *
matrix_identity(size_t size) {
    matrix *instance = Matrix.create(size, size);
    
    while(size--) {
        MATRIX(instance, size, size) = 1;
    }
    
    return instance;
}

// Create from

static
matrix *
matrix_create_from_list(size_t rows, size_t columns, float *values) {
    check(rows > 0 && columns > 0, "Invalid matrix shape");
    check_memory(values);
    
    matrix *instance = malloc(sizeof(matrix));
    instance->type = MATRIX_TYPE;
    
    instance->rows = rows;
    instance->columns = columns;
    instance->vector = Vector.from.floats(rows * columns, values);
    
    return instance;

error:
    return NULL;
}

static
matrix *
matrix_from_vector(vector *v, size_t columns) {
    vector_check(v);
    check(columns, "Matrix should have at least one column");
    matrix *instance = matrix_create(v->size / columns, columns);
    Vector.delete(instance->vector);
    instance->vector = Vector.copy(v);
    
    return instance;

error:
    return NULL;
}

static
matrix *
matrix_from_vectors(vector **vectors, size_t rows, size_t columns) {
    check_memory(vectors);
    check(rows > 0 && columns > 0, "Invalid matrix shape");
    
    matrix *joined = matrix_create(rows, columns);
    
    matrix_foreach(joined) {
        MATRIX(joined, row, column) = VECTOR(vectors[row], column);
    }
    
    return joined;

error:
    return NULL;
}

static
matrix *
matrix_from_cast(void *data, size_t rows, size_t columns) {
    if(IS(data, VECTOR_TYPE)) {
        return matrix_from_vector((vector*)data, columns);
    }
    
    if((int)*(void**)data != 0 && IS(*(void**)data, VECTOR_TYPE)) {
        return matrix_from_vectors((vector**)data, rows, columns);
    }
    
    return matrix_create_from_list(rows, columns, (float*)data);
}

static
matrix *
matrix_diagonal_from_vector(vector *v) {
    vector_check(v);
    matrix *A = matrix_create(v->size, v->size);
    size_t index = v->size;
    
    while(index--) {
        MATRIX(A, index, index) = VECTOR(v, index);
    }
    
    return A;

error:
    return NULL;
}


static
matrix *
matrix_from_csv(csv *file, char *fields[]) {
    check_memory(file);
    check_memory(fields);
    
    size_t index = 0;
    vector **values = malloc(sizeof(vector*));
    
    while(fields[index] != 0) {
        values = realloc(values, (index + 1) * sizeof(vector*));
        char **data = csv_field(file, fields[index]);
        values[index] = Vector.from.strings(file->rows, data);
        
        free( data );
        index++;
    }
    
    matrix *data = Matrix.transpose(matrix_from_vectors(values, index, file->rows));
    
    // Garbage Control
    index = 0;
    while(fields[index] != 0) {
        Vector.delete(values[index]);
        index++;
    }
    free(values);
    
    return data;

error:
    return NULL;
}


/* Getters */
static
vector *
matrix_column_vector(matrix *A, size_t column) {
    matrix_check(A);
    
    size_t vector_size = A->rows;
    vector *column_vector = Vector.create(vector_size);
    for(size_t row = 0; row < vector_size; row++) {
        VECTOR(column_vector, row) = MATRIX(A, row, column);
    }
    
    return column_vector;

error:
    return NULL;
}


/* Transormation */
static
matrix *
matrix_transpose(matrix *instance) {
    matrix_check(instance);
    
    size_t instance_rows = instance->rows;
    size_t transposed_rows = instance->columns;
    
    matrix *transposed = matrix_create(transposed_rows, instance_rows);
    
    size_t rows = transposed_rows;
    
    while(rows--) {
        size_t columns = transposed->columns;
        while(columns--) {
            if(instance_rows == transposed_rows) {
                if(columns > rows) {
                    MATRIX(instance, rows, columns) += MATRIX(instance, columns, rows);
                    MATRIX(instance, columns, rows) = MATRIX(instance, rows, columns) - MATRIX(instance, columns, rows);
                    MATRIX(instance, rows, columns) -= MATRIX(instance, columns, rows);
                } else {
                    columns = 0;
                }
            } else {
                MATRIX(transposed, rows, columns) = MATRIX(instance, columns, rows);
            }
        }
    }
    
    if(instance_rows != transposed_rows) {
        matrix_delete(instance);
        instance = transposed;
        
    } else {
        matrix_delete(transposed);
    }
    
    matrix_check(instance);
    
    return instance;

error:
    return NULL;
}

vector *
vector_transformation_by_matrix(matrix *A, vector *x) {
    matrix_check(A);
    vector_check(x);
    
    vector *transormed_vector = Vector.create(A->rows);
    
    for(size_t column = 0; column < A->columns; column++) {
        vector *column_vector = matrix_column_vector(A, column);
        Vector.add(transormed_vector,
                   Vector.num.mul(column_vector,
                                  VECTOR(x, column)));
        
        Vector.delete(column_vector);
    }
    
    return transormed_vector;

error:
    return NULL;
}


/* Operations */

// Multiplication
static
matrix *
matrix_multiplication(matrix *A, matrix *B) {
    matrix_check(A);
    matrix_check(B);
    matrix *multiplicated = matrix_create(A->rows, B->columns);
    check(A->rows == B->columns, "Matrix sizes doesn't match");

    matrix_foreach(multiplicated) {
        size_t index = A->columns > 1
        ? B->rows
        : A->columns;
        
        while(index--) {
            MATRIX(multiplicated, row, column) += MATRIX(A, row, index) * MATRIX(B, index, column);
        }
    }
    
    Matrix.delete(A);
    A = multiplicated;
    
    return A;
    
error:
    return NULL;
}

static
matrix *
matrix_scalar_multiplication(matrix *A, float scalar) {
    matrix_check(A);
    Vector.mul(A->vector, &scalar);
    
    return A;
    
error:
    return NULL;
}

static
matrix *
matrix_multiplication_cast(matrix *A, void *some_b) {
    if(IS(some_b, MATRIX_TYPE)) {
        return matrix_multiplication(A, (matrix*)some_b);
    }
    
    if(IS(some_b, VECTOR_TYPE)) {
        vector *v_result = vector_transformation_by_matrix(A,
                                                           (vector*)some_b);
        matrix *m_result = Matrix.from(v_result, v_result->size, 1);
        Vector.delete(v_result);
        Matrix.delete(A);
        
        A = m_result;
        return A;
    }
    
    return matrix_scalar_multiplication(A,
                                        ((number*)some_b)->value);
}


// Division
static
matrix *
matrix_division(matrix *A, matrix *B) {
    matrix_check(A);
    matrix_check(B);
    MATRIX_OPERATION(A, B, /);
    
    return A;
    
error:
    return NULL;
}

static
matrix *
matrix_scalar_division(matrix *A, float scalar) {
    matrix_check(A);
    Vector.div(A->vector, &scalar);
    
    return A;
    
error:
    return NULL;
}

static
matrix *
matrix_division_cast(matrix *A, void *some_b) {
    if(IS(some_b, MATRIX_TYPE)) {
        return matrix_division(A, (matrix*)some_b);
    }
    
    if(IS(some_b, VECTOR_TYPE)) {
        return A;
    }
    
    return matrix_scalar_division(A,
                                  ((number*)some_b)->value);
}


// Addition
static
matrix *
matrix_addition(matrix *A, matrix *B) {
    matrix_check(A);
    matrix_check(B);
    MATRIX_OPERATION(A, B, +);
    
    return A;
    
error:
    return NULL;
}

static
matrix *
matrix_scalar_addition(matrix *A, float scalar) {
    matrix_check(A);
    Vector.num.add(A->vector, scalar);
    
    return A;
    
error:
    return NULL;
}

static
matrix *
matrix_addition_cast(matrix *A, void *some_b) {
    if(IS(some_b, MATRIX_TYPE)) {
        return matrix_addition(A, (matrix*)some_b);
    }
    
    if(IS(some_b, VECTOR_TYPE)) {
        return A;
    }
    
    return matrix_scalar_addition(A,
                                  ((number*)some_b)->value);
}


// Subsraction
static
matrix *
matrix_substraction(matrix *A, matrix *B) {
    matrix_check(A);
    matrix_check(B);
    MATRIX_OPERATION(A, B, -);
    
    return A;
    
error:
    return NULL;
}

static
matrix *
matrix_scalar_substraction(matrix *A, float scalar) {
    matrix_check(A);
    Vector.sub(A->vector, &scalar);
    
    return A;
    
error:
    return NULL;
}

static
matrix *
matrix_substraction_cast(matrix *A, void *some_b) {
    if(IS(some_b, MATRIX_TYPE)) {
        return matrix_substraction(A, (matrix*)some_b);
    }
    
    if(IS(some_b, VECTOR_TYPE)) {
        return A;
    }
    
    return matrix_scalar_substraction(A,
                                      ((number*)some_b)->value);
}


// Other operations
static
matrix *
matrix_map(matrix *A, float operation(float)) {
    matrix_check(A);
    matrix_foreach(A) {
        MATRIX(A, row, column) = operation(MATRIX(A, row, column));
    }
    
    return A;
    
error:
    return NULL;
}

/* Properties */
static
float
matrix_sum(matrix *A) {
    matrix_check(A);
    
    return Vector.sum.all(A->vector);
    
error:
    return 0;
}

static
float
matrix_trace(matrix *A) {
    matrix_check(A);
    
    float sum = 0;
    
    size_t index = A->columns;
    while(index--) {
        sum += MATRIX(A, index, index);
    }
    
    return sum;
    
error:
    return 0;
}

static
float
matrix_frobenius_norm(matrix *A) {
    matrix_check(A);
    
    matrix *product = matrix_copy(A);
    MATRIX_OPERATION(product, product, *);
    
    float sum = matrix_sum(product);
    
    matrix_delete(product);
    
    return sqrt(sum);
    
error:
    return 0;
}

static
float
matrix_frobenius_norm_by_trace(matrix *A) {
    matrix_check(A);
    
    matrix *AT = matrix_copy(A);
    Matrix.transpose(AT);
    
    matrix *A_AT = matrix_multiplication(A, AT);
    
    float frobenius = sqrt(matrix_trace(A_AT));
    
    matrix_delete(AT);
    matrix_delete(A_AT);
    
    return frobenius;
    
error:
    return 0;
}


/* Relations */
bool
matrix_is_equal(matrix *A, matrix *B) {
    matrix_check(A);
    matrix_check(B);
    
    return Vector.rel.is_equal(A->vector, B->vector);
    
error:
    return false;
}

/* UI */
static
void
matrix_print(matrix *instance) {
    matrix_check(instance);
    
    size_t previous_row = 0;
    
    printf("\tMatrix: %dx%d\n\t\t[[\t", (int)((int)instance->vector->size / (int)instance->columns), (int)instance->columns);
    
    matrix_foreach(instance) {
        bool is_head_or_tail = row < 5 || row > instance->rows - 5;
        bool is_middle = row == 6;
        bool is_shown = is_head_or_tail || is_middle;
        
        if(row != previous_row && is_shown) {
            if(row > 0) {
                printf("],\n\t\t");
            }
            printf("[\t");
        }
        if(is_head_or_tail) {
            printf("%f\t\t", MATRIX(instance, row, column));
        }
        
        if(is_middle) {
            printf("...\t\t\t");
        }
        
        previous_row = row;
    }
    
    printf("]]");
    
    printf("\n\t\tFrobenius Norm: %f\n", matrix_frobenius_norm(instance));
    
error:
    return;
}

matrix *matrix_create_ui() {
    int rows;
    int columns;
    
    printf("MATRIX size MxN: ");
    scanf("%dx%d", &rows, &columns);
    
    matrix *A = Matrix.create((size_t)rows, (size_t)columns);
    
    matrix_seed(A, 0);
    
    return A;
}


/* Library Structure */
const struct matrix_library Matrix = {
    .create = matrix_create,
    .copy = matrix_copy,
    .reshape = matrix_reshape,
    .delete = matrix_delete,
    
    .print = matrix_print,
    
    .from = matrix_from_cast,
    .csv = matrix_from_csv,
    
    .identity = matrix_identity,
    .diagonal = matrix_diagonal_from_vector,
    
    .seed = matrix_seed,
    .column = matrix_column_vector,
    
    .prop = {
        .sum = matrix_sum,
        .trace = matrix_trace,
        .frobenius_norm = matrix_frobenius_norm_by_trace,
    },
    
    .rel = {
        .is_equal = matrix_is_equal
    },
    
    .add = matrix_addition_cast,
    .sub = matrix_substraction_cast,
    .mul = matrix_multiplication_cast,
    .div = matrix_division_cast,
    
    .transpose = matrix_transpose,
    .map = matrix_map
};

