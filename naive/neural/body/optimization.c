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
vector *
optimization_error_prime(neural_cell *cell) {
    // A(x) - Activation function
    // dA   - Activation slope
    // Z    - transfer function
    // En   - neuron error
    
    size_t axon_dimension = 0;
    vector **errors = malloc(sizeof(vector*));
    // dA = A'(Z)
    vector *cell_activation_prime = Vector.map(Vector.copy(cell->body.transfer),
                                               cell->body.nucleus.activation.derivative);
    
    // Errors from previous neurons
    while(cell->axon[axon_dimension]) {
        neural_cell *axon_cell = cell->axon[axon_dimension];
        matrix *axon_weight = Matrix.seed(Matrix.create(1, cell->body.nucleus.transfer.dimension),
                                          1);
        size_t weight_index = 0;
        while(axon_cell->synapse[weight_index]) {
            if(axon_cell->synapse[weight_index] == cell) {
                for(size_t weight_dimension = 0; weight_dimension < axon_weight->columns; weight_dimension++) {
                    MATRIX(axon_weight, 0, weight_dimension) = MATRIX(axon_cell->body.weight, weight_index, weight_dimension);
                }
                break;
            }
            weight_index++;
        }
        
        vector* transfer_prime = axon_cell->body.nucleus.transfer.prime(axon_weight);
        
        errors = realloc(errors, (axon_dimension + 1) * sizeof(vector*));
        errors[axon_dimension] = Vector.mul(Vector.copy(cell_activation_prime),
                                            axon_cell->body.error);
        Vector.num.mul(errors[axon_dimension], VECTOR(transfer_prime, 0));
        axon_dimension++;
        
        // Garbage Conrol
        Matrix.delete(axon_weight);
        Vector.delete(transfer_prime);
    }
    
    if(axon_dimension == 0) {
        *errors = Vector.copy(cell->body.error);
        axon_dimension = 1;
    }
    
    // Mean error from all axons
    matrix *errors_matrix = Matrix.from(errors, axon_dimension, cell->body.signal->rows);
    vector *error = Vector.create(cell->body.signal->rows);
    
    for(size_t signal = 0; signal < cell->body.signal->rows; signal++) {
        vector *dimension_signals = Matrix.column(errors_matrix, signal);
        VECTOR(error, signal) = Vector.sum.all(dimension_signals) / dimension_signals->size;
    
        Vector.delete(dimension_signals);
    }
    
    // Garbage Conrol
    for(size_t dimension = 0; dimension < axon_dimension; dimension++) {
        Vector.delete(errors[dimension]);
        
    }
    Matrix.delete(errors_matrix);
    Vector.delete(cell_activation_prime);
    free(errors);
    
    // En
    return error;
}

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
    vector *error = optimization_error_prime(cell);
    Vector.delete(cell->body.error);
    cell->body.error = error;
    
    // dW = x * Eo
    matrix *delta_weight = Matrix.mul(Matrix.transpose(Matrix.copy(cell->body.signal)),
                                      error);
    // dW = dW * learnin_rate
    Vector.num.mul(delta_weight->vector, learning_rate);
    
    // dW = dW / m
    Vector.num.div(delta_weight->vector, error->size);
    
    // W = W - dW
    cell->body.weight = Matrix.sub(cell->body.weight,
                                   delta_weight);
    
    // B = B - Eo / m
    cell->body.bias -= learning_rate
                       * Vector.sum.all(error) / error->size;
    
    
    // Garbage Conrol
    Matrix.delete(delta_weight);
    
    return params;
}
