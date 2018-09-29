//
//  optimization.c
//  naive
//
//  Created by Alexandr Kondratyev on 16/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "optimization.h"

static float *     optimization_sgd(void *_cell, float learning_rate, float* params);

/* Library structure */
const struct optimization_library Optimization = {
    .sgd = optimization_sgd
};

static
void
optimization_cell_gradient(neural_cell *cell) {
    // Eo — forward neuron errors
    vector *total_error = Vector.create(cell->context->body.signal->rows);
    
    Vector.delete(cell->context->prime.activation);
    Matrix.delete(cell->context->prime.signal);
    Vector.delete(cell->context->prime.transfer);
    
    vector *activation_derivative = cell->body.nucleus.activation.derivative(cell->context);
    cell->context->prime.activation = activation_derivative;
    
//    cell->context->body.error_derivative = Vector.mul(axon_error,
//                                                     axon_transfer_signal_derivative);
    
    matrix *transfer_derivative_over_weight = cell->body.nucleus.transfer.derivative(cell->context, true);
    matrix *transfer_derivative_over_signal = cell->body.nucleus.transfer.derivative(cell->context, false);
    cell->context->prime.signal = transfer_derivative_over_signal;
    cell->context->prime.transfer = transfer_derivative_over_weight;
    
    // Total Error
    size_t axon_dimension = 0;
    while(cell->axon[axon_dimension]) {
        neural_cell *axon_cell = cell->axon[axon_dimension];
        vector *axon_transfer_signal_derivative = Vector.seed(Vector.create(cell->body.signal->rows),
                                                              1);
                                                              
        size_t cell_in_synapse_index = 0;
        while(axon_cell->synapse[cell_in_synapse_index]) {
            if(axon_cell->synapse[cell_in_synapse_index] == cell) {
                for(size_t sample = 0; sample < axon_transfer_signal_derivative->size; sample++) {
                    VECTOR(axon_transfer_signal_derivative, sample) = MATRIX(axon_cell->context->prime.signal,
                                                                             cell_in_synapse_index,
                                                                             sample);
                }
                break;
            }
            cell_in_synapse_index++;
        }

       
        vector *axon_error = Vector.mul(Vector.copy(axon_cell->context->body.error),
                                        axon_cell->context->prime.activation);
        axon_error = Vector.mul(axon_error,
                                axon_transfer_signal_derivative);
        
        total_error = Vector.add(total_error,
                                 axon_error);
        
        Vector.delete(axon_error);
    }
    
    if(axon_dimension == 0) {
        axon_dimension = 1;
        total_error = Vector.copy(cell->body.error);
    }

    Vector.delete(cell->body.error);
    Vector.delete(cell->context->prime.weight);
    
    cell->body.error = Vector.mul(total_error,
                                  activation_derivative);
    cell->context->body.error = cell->body.error;
    
    cell->context->prime.weight = Vector.mul(Vector.copy(cell->body.error),
                                             transfer_derivative_over_weight);
}

//static
//matrix *
//optimization_error(neural_cell *cell) {
//    // A(x) - Activation function
//    // dA   - Activation slope
//    // Z    - transfer function
//    // En   - neuron error
//    matrix *jacobian;
//    size_t axon_dimension = 0;
//    vector **primes = malloc(sizeof(vector*));
//    // dA = A'(Z)
//    vector *cell_activation_prime = Vector.map_of(Vector.copy(cell->body.transfer),
//                                                  cell->body.nucleus.activation.derivative,
//                                                  NULL);
//
//    // Errors from previous neurons
//    while(cell->axon[axon_dimension]) {
//        neural_cell *axon_cell = cell->axon[axon_dimension];
//        matrix *axon_weight = Matrix.seed(Matrix.create(1, cell->body.nucleus.transfer.dimension),
//                                          1);
//
//        // Get Axon terminal weight
//        size_t weight_index = 0;
//        while(axon_cell->synapse[weight_index]) {
//            if(axon_cell->synapse[weight_index] == cell) {
//                for(size_t weight_dimension = 0; weight_dimension < axon_weight->columns; weight_dimension++) {
//                    MATRIX(axon_weight, 0, weight_dimension) = MATRIX(axon_cell->body.weight, weight_index, weight_dimension);
//                }
//                break;
//            }
//            weight_index++;
//        }
//
//        vector* transfer_prime = axon_cell->body.nucleus.transfer.derivative(cell->body.transfer, axon_weight, axon_cell->body.transfer);
//
//        primes = realloc(primes, (axon_dimension + 1) * sizeof(vector*));
//        matrix *prime = Matrix.mul(Matrix.copy(axon_cell->body.jacobian),
//                                   cell_activation_prime);
//        primes[axon_dimension] = prime->vector;
//        Matrix.delete(prime);
//
//        Vector.num.mul(primes[axon_dimension], VECTOR(transfer_prime, 0));
//
//        axon_dimension++;
//
//        // Garbage Conrol
//        Matrix.delete(axon_weight);
//        Vector.delete(transfer_prime);
//    }
//
//    if(axon_dimension == 0) {
//        axon_dimension = 1;
//        jacobian = Matrix.copy(cell->body.jacobian);
//    } else {
//        jacobian = Matrix.from(primes, axon_dimension, cell->body.signal->rows);
//    }
//
//    // Mean error from all axons
//
////    vector *error = Vector.create(cell->body.signal->rows);
////
////    for(size_t signal = 0; signal < cell->body.signal->rows; signal++) {
////        vector *dimension_signals = Matrix.column(errors_matrix, signal);
////        VECTOR(error, signal) = Vector.sum.all(dimension_signals) / dimension_signals->size;
////
////        Vector.delete(dimension_signals);
////    }
//
//    // Garbage Conrol
//    for(size_t dimension = 0; dimension < axon_dimension; dimension++) {
//        Vector.delete(primes[dimension]);
//    }
//    Vector.delete(cell_activation_prime);
//    free(primes);
//
//    return jacobian;
//}

static
float *
optimization_sgd(void *_cell, float learning_rate, float *params) {
    neural_cell *cell = (neural_cell*)_cell;
    
    // x - input
    // Eo - previous error
    // m - number of examples
    // W - weight
    // dW - delta for weight update
    // B - bias
    
    // Eo
    optimization_cell_gradient(cell);
    
//    Matrix.print(Matrix.mul(Matrix.transpose(Matrix.copy(cell->body.signal)),
//                            jacobian));
//
//    Matrix.delete(cell->body.error);
//    cell->body.error = jacobian;
//
//    // dW = x * Eo
//    matrix *delta_weight = Matrix.mul(Matrix.transpose(Matrix.copy(cell->body.signal)),
//                                      jacobian);
//
//    // dW = dW * learnin_rate
//    Vector.num.mul(delta_weight->vector, learning_rate);
//
//    // dW = dW / m
//    Vector.num.div(delta_weight->vector, jacobian->rows);
//
//    // W = W - dW
//
//    cell->body.weight = Matrix.sub(cell->body.weight,
//                                   delta_weight);
//
//    // B = B - Eo / m
//    cell->body.bias -= learning_rate
//                       * Vector.sum.all(error) / error->size;
    
    
    // Garbage Conrol
//    Matrix.delete(delta_weight);
    
    return params;
}
