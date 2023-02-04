#include "math/vector.h"

#include "error.h"
#include <immintrin.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

int vector_delete(vector *vector);

#define VECTOR_COMMON_SIZE      128
#define COPY_ALIGMENT           32
#define COPY_ALIGMENT_THRESHOLD 32 * 4
#define PRAGMA(x)               _Pragma(#x)
#define VECTOR_OPERATION(result, v, w, expression)                             \
    VECTOR_FOREACH(result)                                                     \
    {                                                                          \
        VECTOR(result, index) = VECTOR(v, index) expression VECTOR(w, index);  \
    }

#define VECTOR_SCALAR_OPERATION(result, v, value, expression)                  \
    VECTOR_FOREACH(result)                                                     \
    {                                                                          \
        VECTOR(result, index) = VECTOR(v, index) expression value;             \
    }

void *copy(void *dest, void *src, size_t bytes)
{
    CHECK(bytes % COPY_ALIGMENT == 0, "Memory should 32 bytes aligned but %ld",
          bytes);
    CHECK(((intptr_t)(dest)&31) == 0, "Check 1 %p", dest);
    CHECK(((intptr_t)(src)&31) == 0, "Check 2 %p", src);

    const __m256i *v_src  = (const __m256i *)(src);
    __m256i       *v_dest = (__m256i *)(dest);
    int64_t        v_nr   = bytes / sizeof(*v_src);
    for (; v_nr > 0; v_nr--, v_src++, v_dest++) {
        const __m256i loaded = _mm256_stream_load_si256(v_src);
        _mm256_stream_si256(v_dest, loaded);
    }
    _mm_sfence();

    return dest;

error:
    return 0;
}

number *integer_create(unsigned int value)
{
    number *instance;

    instance = malloc(sizeof(vector));
    CHECK_MEMORY(instance);

    instance->type    = NNN_INTEGER;
    instance->integer = value;

    return instance;

error:
    return NULL;
}

number *float_create(float value)
{
    number *instance;

    instance = malloc(sizeof(vector));
    CHECK_MEMORY(instance);

    instance->type    = NNN_FLOAT;
    instance->floated = value;

    return instance;

error:
    return NULL;
}

number *double_create(double value)
{
    number *instance;

    instance = malloc(sizeof(vector));
    CHECK_MEMORY(instance);

    instance->type    = NNN_DOUBLE;
    instance->doubled = value;

    return instance;

error:
    return NULL;
}

int number_delete(void *number_ptr)
{
    int     r;
    number *instance;
    CHECK_MEMORY(number_ptr);

    instance = (number *)number_ptr;
    if (NNN_FLOAT >= instance->type) {
        free(instance);
    } else if (NNN_VECTOR == instance->type) {
        r = vector_delete((vector *)instance);
        CHECK(r == 0, "vector_delete() failed");
    }
    // TODO: matrix, tensor, ...

    return 0;

error:
    return 1;
}

vector *vector_create(size_t length)
{
    vector   *instance;
    NNN_TYPE *values;

    instance = malloc(sizeof(vector));
    CHECK_MEMORY(instance);

    values = calloc(length, sizeof(NNN_TYPE));
    CHECK_MEMORY_LOG(values, "Size: %lu", length);

    instance->number.type   = NNN_VECTOR;
    instance->length        = length;
    instance->number.values = values;

    return instance;

error:
    if (instance)
        free(instance);

    return NULL;
}

int vector_delete(vector *vector)
{
    int r;
    CHECK_MEMORY(vector);

    if (NNN_VECTOR == vector->number.type) {
        CHECK_MEMORY(vector->number.values);
        free(vector->number.values);
        free(vector);
    }

    return 0;

error:
    return 1;
}

vector *vector_from_list(size_t length, NNN_TYPE values[])
{
    void     *r;
    vector   *instance;
    size_t    vector_length;
    NNN_TYPE *vector_values;

    CHECK(length, "Vector length should be greater than zero (length=%ld)",
          length);

    instance = malloc(sizeof(vector));
    CHECK_MEMORY(instance);

    vector_length = length * sizeof(NNN_TYPE);
    vector_values = malloc(vector_length);
    CHECK_MEMORY(vector_values);

    if (length % COPY_ALIGMENT == 0 || length > COPY_ALIGMENT_THRESHOLD) {
        r = copy(vector_values, values,
                 vector_length + vector_length % COPY_ALIGMENT);
    } else {
        r = memcpy(vector_values, values, vector_length);
    }
    CHECK(r == vector_values, "copy() %ld bytes failed", length);

    instance->number.type   = NNN_VECTOR;
    instance->length        = length;
    instance->number.values = vector_values;

    return instance;

error:
    if (instance)
        free(instance);
    if (vector_values)
        free(vector_values);

    return NULL;
}

vector *vector_clone(vector *original)
{
    VECTOR_CHECK(original);

    return vector_from_list(original->length, original->number.values);

error:
    return NULL;
}

vector *vector_reshape(vector *instance, size_t length)
{
    void     *r;
    NNN_TYPE *reshaped;

    VECTOR_CHECK(instance);

    reshaped = realloc(instance->number.values, length * sizeof(NNN_TYPE));
    CHECK_MEMORY(reshaped);
    instance->number.values = reshaped;

    if (length > instance->length) {
        memset((NNN_TYPE *)instance->number.values + instance->length, 0,
               (length - instance->length) * sizeof(NNN_TYPE));
    }

    instance->length = length;

    return instance;

error:
    return NULL;
}

#define VECTOR_TYPE_OPERATION(v_block, w_block, size, operation)               \
    case size: {                                                               \
        v##size##sf *block = (v##size##sf *)v_block;                           \
                                                                               \
        if (NNN_DOUBLE >= w->type) {                                           \
            *block = *block operation w->floated;                              \
        } else if (NNN_VECTOR == w->type) {                                    \
            *block = *block operation * (v##size##sf *)w_block;                \
        }                                                                      \
        break;                                                                 \
    }

#define VECTOR_METHOD_OPERATION(name, operation)                               \
    vector *vector_##name(vector *v, number *w)                                \
    {                                                                          \
        int   power;                                                           \
        void *values_added_ptr;                                                \
                                                                               \
        VECTOR_CHECK(v);                                                       \
        NUMBER_CHECK(w);                                                       \
                                                                             \
        /* This code uses the __builtin_clz function, which counts the         \
         * number of leading zero bits in an integer, and bit shifting         \
         * to calculate the nearest power of 2 that is greater than or equal   \
         * to the length of the vector. */                                     \
        power = v->length >= 128                                               \
                    ? 128                                                      \
                    : 1 << (sizeof(int) * 8 - __builtin_clz(v->length));       \
                                                                               \
        PRAGMA(omp for schedule(auto))                                         \
        for (size_t index = 0; index < (v)->length; index = index + power) {   \
            void *block_ptr = (NNN_TYPE *)(v->number.values) + index;          \
                                                                               \
            if (NNN_VECTOR == w->type) {                                       \
                values_added_ptr = (NNN_TYPE *)(w->values) + index;            \
                __builtin_prefetch(values_added_ptr + power, 0, 1);            \
            }                                                                  \
            __builtin_prefetch(block_ptr + power, 1, 1);                       \
                                                                               \
            switch (__builtin_expect(power, VECTOR_COMMON_SIZE)) {             \
                VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 128,        \
                                      operation);                              \
                VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 64,         \
                                      operation);                              \
                VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 32,         \
                                      operation);                              \
                VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 16,         \
                                      operation);                              \
                VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 8,          \
                                      operation);                              \
                VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 4,          \
                                      operation);                              \
                VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 2,          \
                                      operation);                              \
            }                                                                  \
        }                                                                      \
                                                                               \
        return v;                                                              \
                                                                               \
    error:                                                                     \
        return NULL;                                                           \
    }

VECTOR_METHOD_OPERATION(addition, +);
VECTOR_METHOD_OPERATION(substraction, -);
VECTOR_METHOD_OPERATION(multiplication, *);
VECTOR_METHOD_OPERATION(division, /);

vector *vector_addition_func(vector *v, number *w)
{
    int   power;
    void *values_added_ptr;

    VECTOR_CHECK(v);
    NUMBER_CHECK(w);

    /* This code uses the __builtin_clz function, which counts the
     * number of leading zero bits in an integer, and bit shifting
     * to calculate the nearest power of 2 that is greater than or equal to
     * the length of the vector. */
    power = v->length >= 128
                ? 128
                : 1 << (sizeof(int) * 8 - __builtin_clz(v->length));

#pragma omp for schedule(auto)
    for (size_t index = 0; index < (v)->length; index = index + power) {
        void *block_ptr = (float *)(v->number.values) + index;

        if (NNN_VECTOR == w->type) {
            values_added_ptr = (float *)(w->values) + index;
            __builtin_prefetch(values_added_ptr + power, 0, 1);
        }
        __builtin_prefetch(block_ptr + power, 1, 1);

        switch (__builtin_expect(power, VECTOR_COMMON_SIZE)) {
            VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 128, +);
            VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 64, +);
            VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 32, +);
            VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 16, +);
            VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 8, +);
            VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 4, +);
            VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 2, +);
        }
    }

    return v;

error:
    return NULL;
}


float vector_dot_product(vector *v, vector *w)
{
    VECTOR_CHECK(v);
    VECTOR_CHECK(w);

    float product = 0.0f;

    //#pragma omp parallel for simd reduction (+:product)
    VECTOR_FOREACH(v) { product += VECTOR(v, index) * VECTOR(w, index); }

    return product;

error:
    return 0;
}


#define VECTOR_MAP_OPERATION(v_block, size, operation)                         \
    case size: {                                                               \
        v##size##sf *block = (v##size##sf *)v_block;                           \
        for (size_t index = 0; index < size; index++)                          \
            (*block)[index] = operation((*block)[index]);                      \
        break;                                                                 \
    }
vector *vector_map(vector *v, NNN_TYPE operation(NNN_TYPE))
{
    int   power;
    void *values_added_ptr;

    VECTOR_CHECK(v);

    power = v->length >= 128
                ? 128
                : 1 << (sizeof(int) * 8 - __builtin_clz(v->length));

#pragma omp for schedule(static)
    for (size_t index = 0; index < (v)->length; index = index + power) {
        void *block_ptr = (float *)(v->number.values) + index;

        __builtin_prefetch(block_ptr + power, 1, 1);

        switch (__builtin_expect(power, VECTOR_COMMON_SIZE)) {

            VECTOR_MAP_OPERATION(block_ptr, 128, operation);
            VECTOR_MAP_OPERATION(block_ptr, 64, operation);
            VECTOR_MAP_OPERATION(block_ptr, 32, operation);
            VECTOR_MAP_OPERATION(block_ptr, 16, operation);
            VECTOR_MAP_OPERATION(block_ptr, 8, operation);
            VECTOR_MAP_OPERATION(block_ptr, 4, operation);
            VECTOR_MAP_OPERATION(block_ptr, 2, operation);
        }
    }

    return v;

error:
    return NULL;
}

int vector_index_of(vector *v, float needle) {
    VECTOR_CHECK(v);

    VECTOR_FOREACH(v) {
        if(VECTOR(v, index) == needle) {
            return (int)index;
        }
    }

    return -1;

error:
    return -1;
}

NNN_TYPE vector_length(vector *v) {
    VECTOR_CHECK(v);

    return sqrt(vector_dot_product(v, v));

error:
    return 0;
}

void vector_print(vector *instance)
{
    VECTOR_CHECK(instance);

    printf("\tVector size = %d\n\t\t[\t", (int)instance->length);

    VECTOR_FOREACH(instance)
    {
        if (index < 5 || index > instance->length - 5) {
            printf("%.4f,\n\t\t\t", VECTOR(instance, index));
        }

        if (index == 6) {
            printf("...,\n\t\t\t");
        }
    }

    printf("\t\t\t]\n");

error:
    return;
}











/* Different versions */


NNN_TYPE vector_omp_dot_product(vector *v, vector *w)
{
    VECTOR_CHECK(v);
    VECTOR_CHECK(w);

    NNN_TYPE product = 0.0f;

#pragma omp parallel for reduction(+ : product)
    VECTOR_FOREACH(v) { product += VECTOR(v, index) * VECTOR(w, index); }

    return product;

error:
    return 0;
}

#define VECTOR_DOT_OPERATION(v_block, w_block, size, operation)                \
    case size: {                                                               \
        v##size##sf           *block = (v##size##sf *)v_block;                 \
        v##size##sf            product_block;                                  \
        product_block = *block operation * (v##size##sf *)w_block;             \
        PRAGMA(omp parallel for reduction(+:product))                          \
        for (size_t index = 0; index < size; index++)                          \
            product += (float)product_block[index];                            \
        break;                                                                 \
    }

float vector_bloated_dot_product(vector *v, vector *w)
{
    int   power;
    float product = 0.0f;
    void *values_added_ptr;

    VECTOR_CHECK(v);
    NUMBER_CHECK(w);

    /* This code uses the __builtin_clz function, which counts the
     * number of leading zero bits in an integer, and bit shifting
     * to calculate the nearest power of 2 that is greater than or equal to
     * the length of the vector. */
    power = v->length >= 128
                ? 128
                : 1 << (sizeof(int) * 8 - __builtin_clz(v->length));

#pragma omp parallel for reduction(+ : product)
    for (size_t index = 0; index < (v)->length; index = index + power) {
        void *block_ptr = (float *)(v->number.values) + index;

        if (NNN_VECTOR == w->number.type) {
            values_added_ptr = (float *)(w->number.values) + index;
            __builtin_prefetch(values_added_ptr + power, 0, 1);
        }
        __builtin_prefetch(block_ptr + power, 1, 1);

        switch (__builtin_expect(power, VECTOR_COMMON_SIZE)) {
            VECTOR_DOT_OPERATION(block_ptr, values_added_ptr, 128, *);
            VECTOR_DOT_OPERATION(block_ptr, values_added_ptr, 64, *);
            VECTOR_DOT_OPERATION(block_ptr, values_added_ptr, 32, *);
            VECTOR_DOT_OPERATION(block_ptr, values_added_ptr, 16, *);
            VECTOR_DOT_OPERATION(block_ptr, values_added_ptr, 8, *);
            VECTOR_DOT_OPERATION(block_ptr, values_added_ptr, 4, *);
            VECTOR_DOT_OPERATION(block_ptr, values_added_ptr, 2, *);
        }
    }

    return product;

error:
    return 0;
}

vector *vector_naive_map(vector *v, NNN_TYPE operation(NNN_TYPE))
{
    VECTOR_CHECK(v);

    VECTOR_FOREACH(v)
    {
        VECTOR(v, index) = (NNN_TYPE)operation((NNN_TYPE)VECTOR(v, index));
    }

    return v;

error:
    return NULL;
}

vector *vector_omp_map(vector *v, NNN_TYPE operation(NNN_TYPE))
{
    VECTOR_CHECK(v);

#pragma omp parallel for simd
    VECTOR_FOREACH(v)
    {
        VECTOR(v, index) = (NNN_TYPE)operation((NNN_TYPE)VECTOR(v, index));
    }

    return v;

error:
    return NULL;
}


vector *vector_naive_addition(vector *v, number *w)
{
    VECTOR_CHECK(v);
    NUMBER_CHECK(w);

    if (NNN_DOUBLE >= w->type) {
        VECTOR_SCALAR_OPERATION(v, v,
                                (NNN_TYPE)(NNN_INTEGER ? w->integer
                                           : NNN_FLOAT ? w->floated
                                                       : w->doubled),
                                +);
    } else if (NNN_VECTOR == w->type) {
        VECTOR_OPERATION(v, v, w, +);
    }

    return v;

error:
    return NULL;
}

vector *vector_naive_substraction(vector *v, number *w)
{
    VECTOR_CHECK(v);
    NUMBER_CHECK(w);

    if (NNN_DOUBLE >= w->type) {
        VECTOR_SCALAR_OPERATION(v, v,
                                (NNN_TYPE)(NNN_INTEGER ? w->integer
                                           : NNN_FLOAT ? w->floated
                                                       : w->doubled),
                                -);
    } else if (NNN_VECTOR == w->type) {
        VECTOR_OPERATION(v, v, w, -);
    }

    return v;

error:
    return NULL;
}

vector *vector_naive_multiplication(vector *v, number *w)
{
    VECTOR_CHECK(v);
    NUMBER_CHECK(w);

    if (NNN_DOUBLE >= w->type) {
        VECTOR_SCALAR_OPERATION(v, v,
                                (NNN_TYPE)(NNN_INTEGER ? w->integer
                                           : NNN_FLOAT ? w->floated
                                                       : w->doubled),
                                    *);
    } else if (NNN_VECTOR == w->type) {
        VECTOR_OPERATION(v, v, w, *);
    }

    return v;

error:
    return NULL;
}

vector *vector_naive_division(vector *v, number *w)
{
    VECTOR_CHECK(v);
    NUMBER_CHECK(w);

    if (NNN_DOUBLE >= w->type) {
        VECTOR_SCALAR_OPERATION(v, v,
                                (NNN_TYPE)(NNN_INTEGER ? w->integer
                                           : NNN_FLOAT ? w->floated
                                                       : w->doubled),
                                /);
    } else if (NNN_VECTOR == w->type) {
        VECTOR_OPERATION(v, v, w, /);
    }

    return v;

error:
    return NULL;
}


/*   This version is slower on my machine.
    case 64: {
       __m512 block = _mm512_load_ps(block_ptr);
       __m512 values = _mm512_load_ps(values_added_ptr);

       if (NNN_DOUBLE >= w->type) {
           *block = _mm512_add_ps(*block, _mm512_set1_ps(w->floated));
       } else if (NNN_VECTOR == w->type) {
           __m512 result = _mm512_add_ps(block, values);
           _mm512_store_ps(block_ptr, result);
       }
       break;
   }
   VECTOR_TYPE_OPERATION(block_ptr, values_added_ptr, 32, +);
   case 8: {
       __m256 block = _mm256_load_ps(block_ptr);

       v8sf *block_v = (v8sf *)block_ptr;                           \

       if (NNN_DOUBLE >= w->type) {
           *block = _mm256_add_ps(*block, _mm256_set1_ps(w->floated));
       } else if (NNN_VECTOR == w->type) {
           __m256 values = _mm256_load_ps(values_added_ptr);
           __m256 result = _mm256_add_ps(block, values);
           _mm256_store_ps(block_ptr, result);
       }
       break;
   }
*/
