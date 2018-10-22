//
//  activation.c
//  math
//
//  Created by Alexandr Kondratyev on 02/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "activation.h"

static double  sigmoid(double value);
static vector *sigmoid_context(neuron_context *context);
static vector *sigmoid_derivative(neuron_context *context);
static vector *tanh_context(neuron_context *context);
static vector *tanh_derivative(neuron_context *context);
static float   soft_sign(neuron_context *context);
static float   soft_sign_derivative(neuron_context *context);
static float   heaviside_step(neuron_context *context);
static float   heaviside_step_derivative(neuron_context *context);
static float   soft_plus(neuron_context *context);
static float   soft_plus_derivative(neuron_context *context);
static vector *soft_max(neuron_context *context);
static vector *soft_max_derivative(neuron_context *context);
static double relu(double value);
static double relu_derivative(double value);
static vector *relu_context(neuron_context *context);
static vector *relu_derivative_context(neuron_context *context);
static float   leaky_relu(neuron_context *context);
static float   leaky_relu_derivative(neuron_context *context);
static float   elu(neuron_context *context);
static float   elu_derivative(neuron_context *context);
static float   transparent(neuron_context *context);
static float   transparent_derivative(neuron_context *context);

/* Library structure */
const struct activation_library Activation = {
    .sigmoid = {
        .of = sigmoid_context,
        .derivative = sigmoid_derivative,
    },
    .tanh = {
        .of = tanh_context,
        .derivative = tanh_derivative
    },
//    .soft_sign = {
//        .of = soft_sign,
//        .derivative = soft_sign_derivative
//    },
//    .heaviside_step = {
//        .of = heaviside_step,
//        .derivative = heaviside_step_derivative
//    },
//    .soft_plus = {
//        .of = soft_plus,
//        .derivative = soft_plus_derivative
//    },
    .soft_max = {
        .of = soft_max,
        .derivative = soft_max_derivative
    },
    .relu = {
        .of = relu_context,
        .derivative = relu_derivative_context
    },
//    .leaky_relu = {
//        .of = leaky_relu,
//        .derivative = leaky_relu_derivative
//    },
//    .elu = {
//        .of = elu,
//        .derivative = elu_derivative
//    },
//    .transparent = {
//        .of = transparent,
//        .derivative = transparent_derivative
//    },
};


/* Sigmoid */
static
double
sigmoid(double value) {
    return 1. / (1. + exp(value * -1));
}

static
vector *
sigmoid_context(neuron_context *context) {
    return Vector.map(Vector.copy(context->body.transfer),
                      sigmoid);
}


static
vector *
sigmoid_derivative(neuron_context *context) {
    vector *sigmoid_value = sigmoid_context(context);
    vector *one_minus_sigmoid = Vector.num.add(
                                               Vector.num.mul(Vector.copy(sigmoid_value),
                                                              -1),
                                               1);
    sigmoid_value = Vector.mul(sigmoid_value, one_minus_sigmoid);
    Vector.delete(one_minus_sigmoid);
    
    return sigmoid_value;
}


/* ReLU */
static
double
relu(double value) {
    return value > 0
    ? value
    : 0;
}

static
vector *
relu_context(neuron_context *context) {
    return Vector.map(Vector.copy(context->body.transfer),
                      relu);
}

static
double
relu_derivative(double value) {
    return value >= 0
    ? 1
    : 0;
}

static
vector *
relu_derivative_context(neuron_context *context) {
    return Vector.map(Vector.copy(context->body.activation),
                      relu_derivative);
}


/* Tanh */
static
vector *
tanh_context(neuron_context *context) {
    vector_check(context->body.transfer);
    vector *activation = Vector.map(Vector.copy(context->body.transfer),
                                    tanh);
    vector_check(activation);
    return activation;
error:
    return NULL;
}

static
vector *
tanh_derivative(neuron_context *context) {
    vector_check(context->body.activation);
    vector *prime = Vector.map(Vector.copy(context->body.activation),
                               tanh);
    prime = Vector.mul(prime, prime);
    
    prime = Vector.num.add(
                          Vector.num.mul(prime, -1),
                          1);
    vector_check(prime);

    return prime;

error:
    return NULL;
}

/* Softmax */
static
vector *
soft_max(neuron_context *context) {
    size_t number_of_samples = context->body.signal->rows;
    size_t layer_dimension = context->layer.dimension;
    check(number_of_samples && layer_dimension, "Soft max wrong context: n = %zd, l = %zd", number_of_samples, layer_dimension);

    vector *activation = Vector.create(number_of_samples);
    
    for(size_t sample = 0; sample < number_of_samples; sample++) {
        float layer_exp_sum = 0;
        for (vector ***axon = context->layer.axon; *axon; axon++ ){
            vector_check_print(**axon, "Axon vector in layer is broken");
            float layer_axon_value_of_sample = VECTOR(**axon, sample);
            layer_exp_sum += exp(layer_axon_value_of_sample);
        }
        vector_check_print(context->body.activation, "Activation of context is broken");
        float axon_value_of_sample = VECTOR(context->body.activation, sample);
        VECTOR(activation, sample) = exp(axon_value_of_sample) / layer_exp_sum;
    }
    
    return activation;
error:
    return NULL;
}

static
vector *
soft_max_derivative(neuron_context *context) {
    size_t number_of_samples = context->body.activation->size;
    size_t layer_dimension = context->layer.dimension;
    
    vector *prime = Vector.create(number_of_samples);
    
    for(size_t sample = 0; sample < number_of_samples; sample++) {
        float layer_exp_sum = 0;
        for (vector ***axon = context->layer.axon; *axon; axon++ ){
            vector_check_print(**axon, "Axon vector in layer is broken");
            float layer_axon_value_of_sample = VECTOR(**axon, sample);
            layer_exp_sum += exp(layer_axon_value_of_sample);
        }
        vector_check_print(context->body.activation, "Activation of context is broken");

        float axon_value_of_sample = VECTOR(context->body.activation, sample);
        float exp_axon_value = exp(axon_value_of_sample);
        VECTOR(prime, sample) = exp_axon_value * (layer_exp_sum - exp_axon_value)
                                / layer_exp_sum;
    }
    
    return prime;
error:
    return NULL;
}

/* Heaviside Step */
//static
//float
//heaviside_step(neuron_context *context) {
//    return value > 0
//    ? 1
//    : 0;
//}
//
//static
//float
//heaviside_step_derivative(neuron_context *context) {
//    return 0;
//}

/* SoftSign */
//static
//float
//soft_sign(neuron_context *context) {
//    return value / (1 + fabs(value));
//}
//
//static
//float
//soft_sign_derivative(neuron_context *context) {
//    return 1 / pow((1 + fabs(value)), 2);
//}

/* SoftPlus */
//static
//float
//soft_plus(neuron_context *context) {
//    return log(1 + exp(value));
//}
//
//static
//float
//soft_plus_derivative(neuron_context *context) {
//    return 1 / (1 + exp(value * -1));
//}


/* Leaky ReLU */
//static
//float
//leaky_relu(neuron_context *context) {
//    float param = context->variables[0];
//
//    return value > 0
//        ? value
//        : param * value;
//}
//
//static
//float
//leaky_relu_derivative(neuron_context *context) {
//    float param = context->variables[0];
//
//    return value >= 0
//        ? 1
//        : param;
//}
//

/* ELU */
//static
//float
//elu(neuron_context *context) {
//    float param = context->variables[0];
//
//    return value > 0
//            ? value
//            : param * (exp(value) - 1);
//}
//
//static
//float
//elu_derivative(neuron_context *context) {
//    float param = context->variables[0];
//
//    return value >= 0
//            ? 1
//            : elu(value, context) + param;
//}


/* Transparent */
//static
//float
//transparent(neuron_context *context) {
//    return value;
//}
//
//static
//float
//transparent_derivative(neuron_context *context) {
//    return value;
//}
//
