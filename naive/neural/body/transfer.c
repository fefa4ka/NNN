//
//  transfer.c
//  math
//
//  Created by Alexandr Kondratyev on 05/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "transfer.h"

static float       transfer_transparent_vector(vector *input, matrix *weight, float bias);
static vector *     transfer_transparent_matrix(matrix *input, matrix *weight, float bias);
static vector *     transfer_transparent_derivative_vector(vector *predicted, vector *target);



static float       transfer_linear_vector(vector *input, matrix *weight, float bias);
static vector *     transfer_linear_matrix(matrix *input, matrix *weight, float bias);
static vector *     transfer_linear_derivative_vector(vector *predicted, vector *target);
static vector *     transfer_linear_prime_vector(matrix *weigth);


/* Library structure */
const struct transfer_library Transfer = {
    .transparent = {
        .signal = transfer_transparent_vector,
        .samples = transfer_transparent_matrix,
        .derivative = transfer_transparent_derivative_vector,
        
        .dimension = 1
    },
    .linear = {
        .signal = transfer_linear_vector,
        .samples = transfer_linear_matrix,
        .derivative = transfer_linear_derivative_vector,
        .prime = transfer_linear_prime_vector,
        
        .dimension = 1
    }
};

/* Transparent functions */
static
float
transfer_transparent_vector(vector *input, matrix *weight, float bias) {
    return VECTOR(input, 0);
}

static
float
transfer_transparent_derivative_scalar(float predicted, float target) {
    return 0;
}

static
vector *
transfer_transparent_matrix(matrix *input, matrix *weight, float bias) {
    return Matrix.column(input, 0);
}

static
vector *
transfer_transparent_derivative_vector(vector *predicted, vector *target) {
    return Vector.create(predicted->size);
}

static
vector *
transfer_transparent_gradient(matrix *samples,
                              vector *target,
                              vector *predicted) {
    return Vector.create(samples->rows);
}

/* Linear functions */
static
float
transfer_linear_vector(vector *input, matrix *weight, float bias) {
    vector *linear_weight = Matrix.column(weight, 0);
    float prediction = Vector.dot(linear_weight,
                                  input)
    + bias;
    
    Vector.delete(linear_weight);
    
    return prediction;
}

static
float
transfer_linear_prime_scalar(float value) {
    return value;
}

static
float
transfer_linear_derivative_scalar(float predicted, float target) {
    return predicted - target;
}

static
vector *
transfer_linear_matrix(matrix *input, matrix *weight, float bias) {
    vector *linear_weight = Matrix.column(weight, 0);
    matrix *signal = Matrix.mul(Matrix.copy(input), linear_weight);
    vector *prediction = Vector.num.add(Vector.copy(signal->vector),
                                        bias);
    
    Matrix.delete(signal);
    Vector.delete(linear_weight);
    
    return prediction;
}

static
vector *
transfer_linear_derivative_vector(vector *predicted, vector *target) {
    return Vector.sub(Vector.copy(predicted),
                      target);
}

static
vector *
transfer_linear_prime_vector(matrix *weight) {
    return Matrix.column(weight, 0);
}


