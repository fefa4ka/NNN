#pragma once

#include <stdlib.h>

enum nnn_number_type {
    NNN_INTEGER,
    NNN_FLOAT,
    NNN_DOUBLE,
    NNN_VECTOR,
    NNN_MATRIX,
    NNN_TENSOR,
    NNN_RATIONAL,
    NNN_IRRATIONAL,
    NNN_COMPLEX,
    NNN_HYPERCOMPLEX,
    NNN_QUANTERNION,
    NNN_BIG,
    NNN_UNDEFINED
};

#ifndef NNN_TYPE
    #define NNN_TYPE float
#endif

struct nnn_number {
    /* used to store any type of numbers */
    union {
        unsigned int integer;
        float        floated;
        double       doubled;
        void        *values;
    };
    enum nnn_number_type type;
};

struct nnn_vector {
    struct nnn_number number;
    size_t            length;
};

struct nnn_matrix {
    struct nnn_number number;
    size_t            rows;
    size_t            columns;
};

struct nnn_tensor {
    struct nnn_number number;
    size_t            rank;
    size_t           *shape;
};

typedef struct nnn_number number;
typedef struct nnn_vector vector;

#define VECTOR(vector, index)                                                  \
    *((NNN_TYPE *)(((number *)vector)->values) + index)
#define VECTOR_FOREACH(vector)                                                 \
    for (size_t index = 0; index < (vector)->length; index++)
#define VECTOR_VEC_FOREACH(vector, power)                                      \
    for (size_t index = 0; index < (vector)->length; index = index + power)


/* Vector verifying */
#define NUMBER_CHECK_LOG(instance, message, ...)                               \
    {                                                                          \
        CHECK_MEMORY_LOG(instance, message, ##__VA_ARGS__);                    \
        CHECK(((number *)instance)->type >= NNN_INTEGER                        \
                  && ((number *)instance)->type < NNN_UNDEFINED,               \
              "Number with wrong type. " message, ##__VA_ARGS__);              \
    }
#define NUMBER_CHECK(vector) NUMBER_CHECK_LOG(vector, "")
#define VECTOR_CHECK_LOG(vector, message, ...)                                 \
    {                                                                          \
        NUMBER_CHECK_LOG(vector, message, ##__VA_ARGS__);                      \
        CHECK((vector)->length, "Vector length doesn't set. " message,         \
              ##__VA_ARGS__);                                                  \
    }
#define VECTOR_CHECK(vector) VECTOR_CHECK_LOG(vector, "")

/* The v2sf, v4sf, v8sf, etc. are types that are defined using the
 * `__attribute__((vector_size (size)))` attribute, which allows for the
 * creation of custom vector types. These types are used to perform SIMD */
typedef int v2si __attribute__((vector_size(8)));
typedef int v4si __attribute__((vector_size(16)));
typedef int v8si __attribute__((vector_size(32)));
typedef int v16si __attribute__((vector_size(64)));
typedef int v32si __attribute__((vector_size(128)));
typedef int v64si __attribute__((vector_size(256)));
typedef int v128si __attribute__((vector_size(512)));

typedef float v2sf __attribute__((vector_size(8)));
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef float v16sf __attribute__((vector_size(64)));
typedef float v32sf __attribute__((vector_size(128)));
typedef float v64sf __attribute__((vector_size(256)));
typedef float v128sf __attribute__((vector_size(512)));
typedef float v256sf __attribute__((vector_size(1024)));

number *integer_create(unsigned int value);
number *float_create(float value);
number *double_create(double value);
int     number_delete(void *number);

vector *vector_create(size_t length);
vector *vector_from_list(size_t length, NNN_TYPE values[]);
vector *vector_clone(vector *original);
vector *vector_reshape(vector *instance, size_t length);
vector *vector_addition(vector *v, number *w);
vector *vector_substraction(vector *v, number *w);
vector *vector_multiplication(vector *v, number *w);
vector *vector_division(vector *v, number *w);
NNN_TYPE vector_dot_product(vector *v, vector *w);
vector *vector_map(vector *v, NNN_TYPE operation(NNN_TYPE));
int vector_index_of(vector *v, float needle);
void    vector_print(vector *instance);








/* Different versions */
vector *vector_naive_addition(vector *v, number *w);
vector *vector_naive_substraction(vector *v, number *w);
vector *vector_naive_multiplication(vector *v, number *w);
vector *vector_naive_division(vector *v, number *w);
NNN_TYPE vector_omp_dot_product(vector *v, vector *w);
NNN_TYPE vector_bloated_dot_product(vector *v, vector *w);
vector *vector_omp_map(vector *v, NNN_TYPE operation(NNN_TYPE));
vector *vector_naive_map(vector *v, NNN_TYPE operation(NNN_TYPE));
