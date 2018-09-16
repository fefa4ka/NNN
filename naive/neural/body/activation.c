//
//  activation.c
//  math
//
//  Created by Alexandr Kondratyev on 02/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "activation.h"

static float   sigmoid(float value);
static float   sigmoid_derivative(float value);
static float   tanh_derivative(float value);
static float   soft_sign(float value);
static float   soft_sign_derivative(float value);
static float   heaviside_step(float value);
static float   heaviside_step_derivative(float value);
static float   soft_plus(float value);
static float   soft_plus_derivative(float value);
static float   relu(float value);
static float   relu_derivative(float value);
static float   leaky_relu(float value, float param);
static float   leaky_relu_derivative(float value, float param);
static float   elu(float value, float param);
static float   elu_derivative(float value, float param);
static float   transparent(float value);
static float   transparent_derivative(float value);

/* Library structure */
const struct activation_library Activation = {
    .sigmoid = {
        .of = sigmoid,
        .derivative = sigmoid_derivative,
    },
    .tanh = {
        .of = tanh,
        .derivative = tanh_derivative
    },
    .soft_sign = {
        .of = soft_sign,
        .derivative = soft_sign_derivative
    },
    .heaviside_step = {
        .of = heaviside_step,
        .derivative = heaviside_step_derivative
    },
    .soft_plus = {
        .of = soft_plus,
        .derivative = soft_plus_derivative
    },
    .relu = {
        .of = relu,
        .derivative = relu_derivative
    },
    .leaky_relu = {
        .of = leaky_relu,
        .derivative = leaky_relu_derivative
    },
    .elu = {
        .of = elu,
        .derivative = elu_derivative
    },
    .transparent = {
        .of = transparent,
        .derivative = transparent_derivative
    },
};


/* Sigmoid */
static
float
sigmoid(float value) {
    return 1. / (1. + exp(value * -1));
}

static
float
sigmoid_derivative(float value) {
    float sigmoid_value = sigmoid(value);
    
    return sigmoid_value * (1 - sigmoid_value); // Naive implementation
    //    return target / predicted * -1
    //           + (1 - target) / (1 - predicted);
}


/* SoftSign */
static
float
soft_sign(float value) {
    return value / (1 + fabs(value));
}

static
float
soft_sign_derivative(float value) {
    return 1 / pow((1 + fabs(value)), 2);
}


/* Tanh */
static
float
tanh_derivative(float value) {
    return 1 - pow(tanh(value), 2);
}


/* Heaviside Step */
static
float
heaviside_step(float value) {
    return value > 0
    ? 1
    : 0;
}

static
float
heaviside_step_derivative(float value) {
    return 0;
}


/* SoftPlus */
static
float
soft_plus(float value) {
    return log(1 + exp(value));
}

static
float
soft_plus_derivative(float value) {
    return 1 / (1 + exp(value * -1));
}


/* ReLU */
static
float
relu(float value) {
    return value > 0
    ? value
    : 0;
}

static
float
relu_derivative(float value) {
    return value >= 0
    ? 1
    : 0;
}

/* Leaky ReLU */
static
float
leaky_relu(float value, float param) {
    return value > 0
    ? value
    : param * value;
}

static
float
leaky_relu_derivative(float value, float param) {
    return value >= 0
    ? 1
    : param;
}


/* ELU */
static
float
elu(float value, float param) {
    return value > 0
    ? value
    : param * (exp(value) - 1);
}

static
float
elu_derivative(float value, float param) {
    return value >= 0
    ? 1
    : elu(value, param) + param;
}


/* Transparent */
static
float
transparent(float value) {
    return value;
}

static
float
transparent_derivative(float value) {
    return value;
}

