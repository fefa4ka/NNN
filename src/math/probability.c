//
//  probability.c
//  math
//
//  Created by Alexandr Kondratyev on 02/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "probability.h"

// Life Cycle
static P_space probability_space_from_matrix(matrix *samples, char **fields);
static P_space probability_space_from_csv(csv *data, char **fields);
void probability_delete(P_space *P);

// Getters
static float probability_mass_of(P_space *space, char *field, float value);
static float probability_mass_and(P_space *space, char **fields, float *values);
static float probability_mass_or(P_space *space, char **fields, float *values);
static float probability_conditional(P_space *space, char *A_field, float A_value, char *B_field, float B_value);
static float probability_bayes(P_space *space, char *A_field, float A_value, char *B_field, float B_value);
//float probability_density(P_space *space, float a, float b);

// Properties
static float probability_expected_value_conditional(P_space *space, char *expected_field, char *related_field, float value);
static float probability_expected_value(P_space *space, char *field);
// static float probability_matrix_expected_value_of_function(P_space *space, char *field, float operation(float));
static float probability_variance(P_space *space, char *field);
static float probability_covariance(P_space *space, char *field, char *related_field);
static float probability_correlation(P_space *space, char *field, char *related_field);
static size_t probability_get_field_index(P_space *space, char *field);
//float probability_matrix_expected_value(P_space *space);
//float probability_matrix_expected_value_of_function(P_space *space, float operation(float));
//float probability_variance(P_space *space);
//float probability_variance_of_function(P_space *space, float operation(float));

// Helpers
// static P_space *__probability_uniform(P_space *space);
// static P_space *__probability_space_count_events(P_space *space);
// static P_space *__probability_space_count_events_occurs(P_space *space);

const struct probability_library Probability = {
    .delete = probability_delete,
    
    .from = {
        .matrix = probability_space_from_matrix,
        .csv = probability_space_from_csv
    },
    
    
    .mass = {
        .of = probability_mass_of,
        .and = probability_mass_and,
        .or = probability_mass_or,
        .expected = probability_expected_value_conditional
    },
    
    .conditional = probability_conditional,
    .bayes = probability_bayes,
    .expected = probability_expected_value,
    .variance = probability_variance,
    .covariance = probability_covariance,
    .correlation = probability_correlation
};




/* Macros */
#define PROBABILITY_COLUMN(space, field) \
    for(size_t column = 0; column < space->samples->columns; column++) \
        if(strcmp(space->fields[column], field) == 0)

#define PROBABILITY_FOREACH(space, field) \
    PROBABILITY_COLUMN(space, field) \
        for (size_t index = 0; index < space->events[column]->size; index++)

#define PROBABILITY_SUM(space, field, accumulator, expression) \
PROBABILITY_FOREACH(space, field) { \
    float x = VECTOR(space->events[column], index); \
        float Px = VECTOR(space->P[column], index);\
    accumulator += expression; \
}

/* Life Cycle */
static
P_space
probability_space_from_csv(csv *data, char **fields) {
    matrix *samples = Matrix.csv(data, fields);
    
    return probability_space_from_matrix(samples, fields);
}

static
P_space
probability_space_from_matrix(matrix *samples, char **fields) {
    size_t space_width_size = sizeof(vector*) * samples->columns;
    char **_fields = malloc(samples->columns * sizeof(char*));

    for(size_t index = 0; index < samples->columns; index++) {
        _fields[index] = strdup(fields[index]);
    }
    
    P_space space = {
        .type = PROBABILITY_TYPE,
        .fields = _fields,
        .samples = Matrix.copy(samples),
        .events = malloc(space_width_size),
        .occurs = malloc(space_width_size),
        .P = malloc(space_width_size),
        .variance = malloc(samples->columns * sizeof(float)),
        .covariance = Matrix.create(samples->columns, samples->columns),
        .correlation = Matrix.create(samples->columns, samples->columns)
    };
    
    return space;
}

void probability_delete(P_space *space) {
    for(size_t index = 0; index < space->samples->columns; index++) {
        Vector.delete(space->events[index]);
        Vector.delete(space->P[index]);
        Vector.delete(space->occurs[index]);
        free(space->fields[index]);
    }
    
    Matrix.delete(space->samples);
    Matrix.delete(space->covariance);
    Matrix.delete(space->correlation);
    
    free(space->events);
    free(space->occurs);
    free(space->P);
    free(space->variance);
    free(space->fields);
}


/* Count Helpers */
// Get index of column
static
size_t
probability_get_field_index(P_space *space, char *field) {
    size_t index = 0;
    while(space->fields[index]){
        if(strcmp(space->fields[index], field) == 0) {
            return index;
        }
        
        index++;
    }
    
    return 0;
}

// Count helpers
/* Unused
static
P_space *
__probability_uniform(P_space *space) {
    for(size_t column = 0; column < space->samples->columns; column++) {
        space->P[column] = Vector.num.div(Vector.copy(space->occurs[column]),
                                          space->samples->rows);
        space->variance[column] = probability_variance(space, space->fields[column]);
        
        // Covariance matrix
        for(size_t related_column = 0; related_column <= column; related_column++) {
            float covariation = probability_covariance(space, space->fields[column], space->fields[related_column]);;
            
            MATRIX(space->covariance, column, related_column) = covariation;
            MATRIX(space->covariance, related_column, column) = covariation;
            
            float correlation = probability_correlation(space, space->fields[column], space->fields[related_column]);
            MATRIX(space->correlation, column, related_column) = correlation;
            MATRIX(space->correlation, related_column, column) = correlation;
        }
    }
    
    return space;
}

static
P_space *
__probability_space_count_events(P_space *space) {
    matrix *samples = space->samples;
    
    for(size_t index = 0; index < samples->columns; index++) {
        vector *column_data = Matrix.column(samples, index);
        space->events[index] = Vector.prop.uniq(column_data);
        space->occurs[index] = Vector.create(space->events[index]->size);
        
        Vector.delete(column_data);
    }

    space = __probability_space_count_events_occurs(space);
    space = __probability_uniform(space);
    
    return space;
}


static
P_space *
__probability_space_count_events_occurs(P_space *space) {
    matrix_foreach(space->samples) {
        int event_index = Vector.prop.index_of(space->events[column], MATRIX(space->samples, row, column));        
        VECTOR(space->occurs[column], (size_t)event_index) += 1;
    }
    
    return space;
}
*/


/* Probability Mass Function */
static
float
probability_mass_of(P_space *space, char *field, float value) {
    PROBABILITY_COLUMN(space, field) {
        return VECTOR(space->P[column], Vector.prop.index_of(space->events[column], value));
    }
    
    return 0;
}

// Probability mass function of A and B ... and N .. . fields = { 'param_one', 'second_feature', NULL }, values = { 1, 2, 3 }
static
float
probability_mass_and(P_space *space, char **fields, float *values) {
    size_t occur = 0;
    
    for(size_t index = 0; index < space->samples->rows; index++) {
        bool is_suitable = false;
        
        for(size_t column = 0; column < space->samples->columns; column++) {
            size_t field_index = 0;
            bool is_breaked = false;
            while(fields[field_index]) {
                if(strcmp(space->fields[column], fields[field_index]) == 0) {
                    if(MATRIX(space->samples, index, column) == values[field_index]) {
                        is_suitable = true;
                    } else {
                        is_suitable = false;
                        is_breaked = true;
                        break;
                    }
                }
                field_index++;
            }
            
            if(is_breaked) {
                is_suitable = false;
                break;
            }
        }
        
        if(is_suitable) {
            occur++;
        }
    }
    
    return (float)occur / (float)space->samples->rows;
}

static
float
probability_mass_or(P_space *space, char **fields, float *values) {
    float P = 0;
    
    for(size_t column = 0; column < space->samples->columns; column++) {
        size_t field_index = 0;
        while(fields[field_index]) {
            if(strcmp(space->fields[column], fields[field_index]) == 0) {
                P += VECTOR(space->P[column], Vector.prop.index_of(space->events[column], values[field_index]));
            }
            field_index++;
        }
    }
    
    return P;
}

/* Conditional probability */
static
float
probability_conditional(P_space *space, char *A_field, float A_value, char *B_field, float B_value) {
    char *fields[] = { A_field, B_field, NULL };
    float values[] = { A_value, B_value };
    
    float P_AB = probability_mass_and(space, fields, values);
    float P_B = probability_mass_of(space, B_field, B_value);
    
    return P_AB / P_B;
}

static
float
probability_bayes(P_space *space, char *A_field, float A_value, char *B_field, float B_value) {
    float P_BA = probability_conditional(space, B_field, B_value, A_field, A_value);
    float P_A = probability_mass_of(space, A_field, A_value);
    float P_B = probability_mass_of(space, B_field, B_value);
    
    return P_A * P_BA / P_B;
}

/* Expectation */
static
float
probability_expected_value(P_space *space, char *field) {
    float mu = 0;
    
    PROBABILITY_SUM(space, field, mu, x * Px);
    
    return mu;
}

static
float
probability_expected_value_conditional(P_space *space, char *expected_field, char *related_field, float value) {
    char *fields[] = { expected_field, related_field, NULL };
    float expected_value = 0;
    vector *expected = NULL;
    vector *related = NULL;
    
    for(size_t column = 0; column < space->samples->columns; column++) {
        if(strcmp(space->fields[column], expected_field) == 0) {
            expected = Vector.copy(space->events[column]);
        }
        if(strcmp(space->fields[column], related_field) == 0) {
            related = space->events[column];
        }
    }
    
    if(expected && related) {
        for(size_t index = 0; index < expected->size; index++) {
            float values[] = { VECTOR(expected, index), value };
            
            VECTOR(expected, index) *= probability_mass_and(space,
                                                            fields,
                                                            values);
        }
        
        expected_value = Vector.sum.all(expected);
    }
    
    Vector.delete(expected);
    
    return expected_value;
}


/* Variance */
static
float
probability_variance(P_space *space, char *field)
{
    float mu = probability_expected_value(space, field);
    float sigma2 = 0;
    
    PROBABILITY_SUM(space, field, sigma2, pow(x - mu, 2) * Px);
    
    return sigma2;
}

static
float
probability_covariance(P_space *space, char *field, char *related_field)
{
    char *fields[] = { field, related_field, NULL };
    vector *origin = space->events[probability_get_field_index(space, field)];
    vector *related = space->events[probability_get_field_index(space, related_field)];
    
    float mu_field = probability_expected_value(space, field);
    float mu_related = probability_expected_value(space, related_field);
    
    float covariance = 0;
    
    //#pragma omp parallel for reduction (+:covariance) 
    vector_foreach(origin) {
        float x = VECTOR(origin, index);
        float y = VECTOR(related, index);
        float values[] = {x, y};
        float P = probability_mass_and(space, fields, values);
        covariance += (x - mu_field) * (y - mu_related) * P;
    }
    
    return covariance;
}

static
float
probability_correlation(P_space *space, char *field, char *related_field)
{
    size_t field_index = probability_get_field_index(space, field);
    size_t related_field_index = probability_get_field_index(space, related_field);
    float mu_field = space->variance[field_index];
    float mu_related = space->variance[related_field_index];
    
    float correlation = MATRIX(space->covariance, field_index, related_field_index) / sqrt(mu_field * mu_related);
    
    return correlation;
}


/* Unused
static
float
probability_expected_value_of_function(P_space *space, char *field, float operation(float)) {
    float mu = 0;

    PROBABILITY_SUM(space, field, mu, operation(x) * Px);

    return mu;
}



float probability_density(P_space *space, float a, float b) {
    float probability = 0;

    float *proper_list = malloc(space->items * sizeof(float));
    size_t proper_size = 0;

    PROBABILITY_FOREACH(space) {
        if(VECTOR(space->events, index) > a && VECTOR(space->events, index) < b) {
            proper_list[proper_size++] = VECTOR(space->events, index);
        }
    }

    proper_list = realloc(proper_list, proper_size * sizeof(float));

    merge_sort(proper_list, 0, proper_size);

    while(--proper_size) {
        float a = proper_list[proper_size - 1];
        float b = proper_list[proper_size];
        float Pb = probability_uniform_of(space, b);

        probability += Pb * (b - a);
    }

    return probability;
}

// Properties 
float probability_matrix_expected_value(P_space *space) {
    float mu = 0;

    PROBABILITY_SUM(space, mu, x * Px);

    return mu;
}



float probability_variance_of_function(P_space *space, float operation(float)) {
    float sigma2 = 0;
    float mu = probability_matrix_expected_value_of_function(space, operation);

    PROBABILITY_SUM(space, sigma2, pow(operation(x) - mu, 2) * Px);

    return sigma2;
}

*/
