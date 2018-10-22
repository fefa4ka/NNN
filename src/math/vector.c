
//
//  vector.c
//  math
//
//  Created by Alexandr Kondratyev on 01/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "vector.h"

// Life Cycle
static number *      number_create(float value);
static vector *      vector_create(size_t size);
static vector *      vector_create_from_list(size_t size, float *values);
static vector *      vector_create_from_list_char(size_t size, char **values);
static vector *      vector_copy(vector *original);
static vector *      vector_reshape(vector *instance, size_t size);
static void          vector_delete(void *instance);

// Data
static vector *      vector_seed(vector *instance, float default_value);
static vector_hash * vector_hash_list(size_t size, char **list);

// Operations
static vector *      vector_addition_cast(vector *v, void *term);
static vector *      vector_addition(vector *v, vector *w);
static vector *      vector_scalar_addition(vector *v, float scalar);

static vector *      vector_substraction_cast(vector *v, void *subtrahend);
static vector *      vector_substraction(vector *v, vector *w);
static vector *      vector_scalar_substraction(vector *v, float scalar);

static vector *      vector_multiplication_cast(vector *v, void *factor);
static vector *      vector_vector_multiplication(vector *v, vector *w);
static vector *      vector_scalar_multiplication(vector *v, float scalar);

static vector *      vector_division_cast(vector *v, void *divider);
static vector *      vector_vector_division(vector *v, vector *w);
static vector *      vector_scalar_division(vector *v, float scalar);

static float         vector_dot_product(vector *v, vector *w);
static vector *      vector_map(vector *v, double operation(double));
static vector *      vector_map_param(vector *v, float operation(float, float*), float *operation_params);

// Properties
static int           vector_index_of(vector *v, float needle);
static float         vector_length(vector *v);
static float         vector_l_norm(vector *v, int p);
static float         vector_max_norm(vector *v);
static vector *      vector_unit(vector *v);
static vector *      vector_uniq(vector *instance);
// Sums
static float        vector_sum(vector *v);
static float        vector_sum_to(vector *v, size_t to_index);
static float        vector_sum_between(vector *v, size_t from_index, size_t to_index);

// Relations
static float         vector_angle(vector *v, vector *w);
static enum bool     vector_is_perpendicular(vector *v, vector *w);
static enum bool     vector_is_equal(vector *v, vector *w);

// UI
static void          vector_print(vector *instance);


/* Library Structure */
const struct vector_library Vector = {
    .create = vector_create,
    .copy = vector_copy,
    .reshape = vector_reshape,
    .seed = vector_seed,
    .delete = vector_delete,
    
    .print = vector_print,
    
    .from = {
        .number = number_create,
        .floats = vector_create_from_list,
        .strings = vector_create_from_list_char,
        .hash = vector_hash_list
    },
    
    .prop = {
        .index_of = vector_index_of,
        .length = vector_length,
        .unit = vector_unit,
        .l_norm = vector_l_norm,
        .max_norm = vector_max_norm,
        .uniq = vector_uniq
    },
    
    .sum = {
        .all = vector_sum,
        .to = vector_sum_to,
        .between = vector_sum_between
    },
    
    .rel = {
        .is_equal = vector_is_equal,
        .angle = vector_angle,
        .is_perpendicular = vector_is_perpendicular
    },
    
    // Operations
    .add = vector_addition_cast,
    .sub = vector_substraction_cast,
    .mul = vector_multiplication_cast,
    .div = vector_division_cast,
    
    .num = {
        .add = vector_scalar_addition,
        .sub = vector_scalar_substraction,
        .mul = vector_scalar_multiplication,
        .div = vector_scalar_division
    },
    
    .dot = vector_dot_product,
    .map = vector_map,
    .map_of = vector_map_param
};


/* Macros */
#define VECTOR_OPERATION(result, v, w, expression)                                    \
    vector_foreach(result)                                                            \
    {                                                                                 \
        VECTOR(result, index) = VECTOR(v, index) expression VECTOR(w, index);         \
        check(VECTOR(result, index) == VECTOR(result, index),                         \
              "Operation %f " #expression " %f is %f", VECTOR(v, index), VECTOR(w, index), VECTOR(result, index)); \
    }

#define VECTOR_SCALAR_OPERATION(result, v, scalar, expression)                                           \
    vector_foreach(result)                                                                               \
    {                                                                                                    \
        VECTOR(result, index) = VECTOR(v, index) expression scalar;                                      \
        check(VECTOR(result, index) == VECTOR(result, index),                                            \
              "Operation %f " #expression " %f is %f", VECTOR(v, index), scalar, VECTOR(result, index)); \
    }

/* Life Cycle */
static
number *
number_create(float value) {
    number *instance = malloc(sizeof(number));
    instance->type = NUMBER_TYPE;
    instance->value = value;
    return instance;
}

static
vector *
vector_create(size_t size) {
    vector *instance = malloc(sizeof(vector));
    float *values = calloc(size, sizeof(float));
    check_memory(instance);
    check_memory(values);

    instance->type = VECTOR_TYPE;
    instance->size = size;
    instance->values = values;
    
    return instance;
error:
    return NULL;
}

static
vector *
vector_create_from_list(size_t size, float values[]) {
    check(size, "Vector size should be greater than zero.");
    
    vector *instance = malloc(sizeof(vector));
    check_memory(instance);
    
    size_t vector_size = size * sizeof(float);
    float *vector_values = malloc(vector_size);
    memcpy(vector_values, values, vector_size);
    check_memory(vector_values);
    
    instance->type = VECTOR_TYPE;
    instance->size = size;
    instance->values = vector_values;

    return instance;

error:
    return NULL;
}

static
vector *
vector_create_from_list_char(size_t size, char **values) {
    check(size, "Vector size should be greater than zero.");
    
    vector *instance = calloc(1, sizeof(vector));
    enum bool is_hash = false;
    check_memory(instance);
    
    size_t vector_size = size * sizeof(float);
    instance->type = VECTOR_TYPE;
    instance->size = size;
    instance->values = malloc(vector_size);
    check_memory(instance->values);
    
    vector_foreach(instance) {
        if(atof(values[index]) == false) {
            is_hash = true;
            break;
        }
        VECTOR(instance, index) = atof(values[index]);
    }
    
    if(is_hash) {
        vector_hash *hash = Vector.from.hash(size, values);
        Vector.delete(instance);
        instance = Vector.copy(hash->index);
        Vector.delete(hash);
    }
    
    return instance;

error:
    return NULL;
}

static
vector *
vector_uniq(vector *instance) {
    check_memory(instance);
    size_t size = 0;
    float *uniq = uniq_floats(instance->values, instance->size, &size);
    check_memory(uniq);

    return vector_create_from_list(size, uniq);
error:
    return NULL;
}

static
vector *
vector_copy(vector *original) {
    vector_check(original);
    
    return vector_create_from_list(original->size, original->values);

error:
    return NULL;
}

static
vector *
vector_reshape(vector *instance, size_t size) {
    vector_check(instance);
    check(size, "Vector size should be greater than zero.");
    
    instance->values = realloc(instance->values, size * sizeof(float));
    
    if(size > instance->size) {
        memset(instance->values + instance->size, 0, (size - instance->size) * sizeof(float));
    }
    
    instance->size = size;
    
    return instance;

error:
    return NULL;
}

static
void
vector_delete(void *instance) {
    check_memory(instance);
    
    if (IS(instance, VECTOR_TYPE))
    {
        vector *vec = (vector*)instance;
        free(vec->values);
        free(vec);
    }
    
    if(IS(instance, VECTOR_HASH_TYPE)) {
        vector_hash *hash = (vector_hash*)instance;
        Vector.delete(hash->index);
        free(hash->keys);
        free(hash);
    }

error:
    return;
}

/* Data */
static
vector *
vector_seed(vector *instance, float default_value) {
    vector_check(instance);
    
    vector_foreach(instance) {
        if(default_value) {
            VECTOR(instance, index) = default_value;
        } else {
            VECTOR(instance, index) = random_range(-1, 1);
        }
    }
    
    return instance;

error:
    return NULL;
}

static
vector_hash *
vector_hash_list(size_t size, char **list) {
    vector_hash *hash = malloc(sizeof(vector_hash));
    hash->type = VECTOR_HASH_TYPE;
    hash->keys = uniq_strings(list, size, &hash->size);
    hash->index = vector_create(size);
    
    vector_foreach(hash->index) {
        size_t key_index = 0;
        while(hash->keys[key_index]) {
            if(strcmp(hash->keys[key_index], list[index]) == 0) {
                VECTOR(hash->index, index) = key_index;
            }
            key_index++;
        }
    }
    
    return hash;
}


/* Operations */

// Addition
static
vector *
vector_addition_cast(vector *v, void *term) {
    vector_check(v);
    
    if(IS(term, VECTOR_TYPE)) {
        return vector_addition(v, (vector *)term);
    }
    
    return vector_scalar_addition(v,
                                  ((number*)term)->value);

error:
    return NULL;
}

static
vector *
vector_addition(vector *v, vector *w) {
    vector_check(v);
    vector_check(w);
    check(v->size == w->size, "Vector size doesn't match");
    VECTOR_OPERATION(v, v, w, +);

    return v;

error:
    return NULL;
}

static
vector *
vector_scalar_addition(vector *v, float scalar) {
    vector_check(v);
    VECTOR_SCALAR_OPERATION(v, v, scalar, +);
    
    return v;

error:
    return NULL;
}


// Substraction
static
vector *
vector_substraction_cast(vector *v, void *subtrahend) {
    vector_check(v);
    
    if(IS(subtrahend, VECTOR_TYPE)) {
        return vector_substraction(v, (vector *)subtrahend);
    }
    
    return vector_scalar_substraction(v,
                                      ((number*)subtrahend)->value);

error:
    return NULL;
}

static
vector *
vector_substraction(vector *v, vector *w) {
    vector_check(v);
    vector_check(w);
    
    VECTOR_OPERATION(v, v, w, -);
    
    return v;

error:
    return NULL;
}

static
vector *
vector_scalar_substraction(vector *v, float scalar) {
    vector_check(v);
    VECTOR_SCALAR_OPERATION(v, v, scalar, -);
    
    return v;

error:
    return NULL;
}

// Multiplication
static
vector *
vector_multiplication_cast(vector *v, void *factor) {
    vector_check(v);
    return vector_vector_multiplication(v, (vector *)factor);
    
error:
    return NULL;
}

static
vector *
vector_vector_multiplication(vector *v, vector *w) {
    vector_check(v);
    vector_check(w);
    check(v->size == w->size, "Vectors size doesn't match");
    VECTOR_OPERATION(v, v, w, *);

    return v;

error:
    return NULL;
}

static
vector *
vector_scalar_multiplication(vector *v, float scalar) {
    vector_check(v);
    VECTOR_SCALAR_OPERATION(v, v, scalar, *);
    
    return v;

error:
    return NULL;
}


// Division
static
vector *
vector_division_cast(vector *v, void *divider) {
    vector_check(v);
    
    if(IS(divider, VECTOR_TYPE)) {
        return vector_vector_division(v, (vector *)divider);
    }
    
    return vector_scalar_division(v,
                                  ((number*)divider)->value);
    
error:
    return NULL;
}

static
vector *
vector_vector_division(vector *v, vector *w) {
    vector_check(v);
    VECTOR_OPERATION(v, v, w, /);
    
    return v;
    
error:
    return NULL;
}

static
vector *
vector_scalar_division(vector *v, float scalar) {
    vector_check(v);
    VECTOR_SCALAR_OPERATION(v, v, scalar, /);
    
    return v;
    
error:
    return NULL;
}

// Dot
static
float
vector_dot_product(vector *v, vector *w) {
    vector_check(v);
    vector_check(w);
    
    float product = 0;
    
    vector_foreach(v) {
        product += VECTOR(v, index) * VECTOR(w, index);
    }
    
    return product;
    
error:
    return 0;
}

/* Map */
static
vector *
vector_map(vector *v, double operation(double)) {
    vector_check(v);

    vector_foreach(v) {
        VECTOR(v, index) = (float)operation((double)VECTOR(v, index));
    }
    
    return v;
    
error:
    return NULL;
}

static
vector *
vector_map_param(vector *v, float operation(float, float*), float *operation_params) {
    vector_check(v);

    vector_foreach(v) {
        VECTOR(v, index) = operation(VECTOR(v, index), operation_params);
    }
    
    return v;
    
error:
    return NULL;
}


/* Properties */
static
int
vector_index_of(vector *v, float needle) {
    vector_check(v);

    vector_foreach(v) {
        if(VECTOR(v, index) == needle) {
            return (int)index;
        }
    }
    
    return -1;

error:
    return -1;
}

static
float
vector_length(vector *v) {
    vector_check(v);

    return sqrt(vector_dot_product(v, v));

error:
    return 0;
}

// Sum
static
float
vector_sum(vector *v) {
    vector_check(v);
    
    float sum = 0;
    
    vector_foreach(v) {
        sum += VECTOR(v, index);
    }
    
    return sum;
    
error:
    return 0;
}

static
float
vector_sum_to(vector *v, size_t to_index) {
    vector_check(v);
    
    float sum = 0;
    
    vector_foreach(v) {
        sum += VECTOR(v, index);
        if(index == to_index) {
            return sum;
        }
    }
    
    return sum;

error:
    return 0;
}

static
float
vector_sum_between(vector *v, size_t from_index, size_t to_index) {
    vector_check(v);

    float sum = 0;
    
    for(size_t index = from_index; index < to_index; index++) {
        sum += VECTOR(v, index);
    }
    
    return sum;
    
error:
    return 0;
}

// Unit vector
static
vector *
vector_unit(vector *v) {
    vector_check(v);

    float length = vector_length(v);
    
    return vector_scalar_division(v, length);
    
error:
    return NULL;
}

// Norm
static
float
vector_l_norm(vector *v, int p) {
    vector_check(v);
    check(p, "P = 0 for L_norm");
    
    float l_norm = 0;
    
    size_t index = v->size;
    while (index--)
    {
        l_norm += pow(fabs(VECTOR(v, index)), p);
    }
    
    l_norm = pow(l_norm, (float)1 / p);
    
    return l_norm;
    
error:
    return 0;
}

static
float
vector_max_norm(vector *v) {
    vector_check(v);

    float max = 0;
    
    size_t index = v->size;
    while(index--) {
        float value = fabs(VECTOR(v, index));
        if (value > max) {
            max = value;
        }
    }
    
    return max;
    
error:
    return 0;
}

/* Relations */
static
float
vector_angle(vector *v, vector *w) {
    vector_check(v);
    vector_check(w);
    
    float cosine = vector_dot_product(v, w) / (vector_length(v) * vector_length(w));
    
    float angle_in_degrees = acos(cosine) * 180 / PI;
    
    return angle_in_degrees;
    
error:
    return 0;
}

static
enum bool
vector_is_perpendicular(vector *v, vector *w) {
    vector_check(v);
    vector_check(w);

    float dot_product = vector_dot_product(v, w);
    
    return dot_product == 0
    ? true
    : false;
    
error:
    return 0;
}

static
enum bool
vector_is_equal(vector *v, vector *w) {
    vector_check(v);
    vector_check(w);
    
    enum bool is_equal = v->size == w->size
    && memcmp(v->values, w->values, v->size) == 0;
    
    return is_equal;

error:
    return false;
}


/* UI */
static
void
vector_print(vector *instance) {
    vector_check(instance);
    
    printf("\tVector size = %d\n\t\t[\t", (int)instance->size);
    
    vector_foreach(instance) {
        if(index < 5 || index > instance->size - 5) {
            printf("%.4f,\n\t\t\t", VECTOR(instance, index));
        }
        
        if(index == 6) {
            printf("...,\n\t\t\t");
        }
    }
    
    printf("\t\t\t]\n");

    
error:
    return;
}

vector *
vector_create_ui() {
    int size;
    
    printf("Vector size I: ");
    scanf("%d", &size);
    
    vector *v = vector_create((size_t)size);
    
    if(size < 10) {
        vector_foreach(v) {
            int number;
            
            printf("%d: ", (int)index);
            scanf("%d", &number);
            
            v->values[index] = (float)number;
        }
    } else {
        vector_seed(v, 0);
    }
    
    return v;
}




