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
optimization_back_propagation(neural_cell *cell) {
    struct neuron_state *body = &cell->context->body;
    struct neuron_state *prime = &cell->context->prime;
    
    // Eo = C'(O-y) * ... if output layer
    vector *base_error = *cell->axon
                            ? Vector.create(body->signal->rows)
                            : Vector.copy(cell->body.error);
    vector_check_print(base_error, "Base error (Eo) vector for back propagate is broken");
    vector_values_check(base_error);

    // Eo — error from the neurons in front
    size_t axon_dimension = 0;
    for(neural_cell *axon = cell->axon[0]; cell->axon[axon_dimension]; axon = cell->axon[axon_dimension]) {
        neuron_ccheck(axon, "Broken cell in axon terminal");
        size_t signal_dimension = axon->body.signal->columns;
        
        for(size_t index = 0; index < signal_dimension; index++) {
            if(axon->synapse[index] == cell) {
                // En = Ei * Wi
                struct neuron_state front_prime = axon->context->prime;
                vector *front_error = front_prime.error;
                vector_values_check(front_error);
                vector_check_print(front_error, "Error vector of front neuron %zdx%zd", axon->coordinates.layer, axon->coordinates.position);
                front_error = Vector.num.mul(front_error, MATRIX(front_prime.weight, index, 0));
                // Eo += En
                vector_values_check(base_error);
                base_error = Vector.add(base_error, front_error);
                vector_values_check(base_error);
                axon_dimension++;
            }
        }
    }

    if(*cell->axon) {
        // Eo = SUM(En) / Number of cells in axon terminal
        check(axon_dimension, "Axon dimension is 0 when pointer to axon exists");
        base_error = Vector.num.div(base_error, axon_dimension);
        vector_values_check(base_error);
    }

    vector_check(base_error);
    

    // Z'(X)
    matrix *transfer_derivative_over_signal = cell->body.nucleus.transfer.derivative(cell->context, false);
    matrix_check(transfer_derivative_over_signal);
    
    // Z'(W)? - if last layer, W is 1
    matrix *transfer_derivative_over_weight = *cell->axon
        ? Matrix.seed(Matrix.create(body->weight->rows, body->weight->columns), 1)
        : cell->body.nucleus.transfer.derivative(cell->context, true);
    matrix_check(transfer_derivative_over_weight);
    
    // R'(Z)
    vector *activation_derivative = cell->body.nucleus.activation.derivative(cell->context);
    vector_check(activation_derivative);
    vector_values_check(activation_derivative);
    vector_values_check(base_error);
    // E = Eo * R'(Z) * ... - current layer error
    base_error = Vector.mul(base_error, activation_derivative);
    vector_values_check(base_error);
    
    // C'(W) = E * Z'(X)
    matrix *cost_weight_prime = Matrix.copy(body->weight);
    vector_values_check(transfer_derivative_over_signal->vector);
    matrix_foreach(cost_weight_prime) {
        float term = MATRIX(transfer_derivative_over_signal, row, column);
        
        vector *cw_prime = Vector.num.mul(Vector.copy(base_error),
                                          term);
        MATRIX(cost_weight_prime, row, column) = Vector.sum.all(cw_prime) / cw_prime->size;
        Vector.delete(cw_prime);
    }
    matrix_check(cost_weight_prime);
    vector_values_check(cost_weight_prime->vector);
    // Garbage Control
    if(prime->activation) {
        Vector.delete(prime->activation);
        Matrix.delete(prime->signal);
        Vector.delete(prime->transfer);
        Vector.delete(prime->weight);
        Vector.delete(prime->error);
    }
    
    // Assign to cell context
    prime->signal = transfer_derivative_over_signal;
    prime->transfer = Vector.copy(transfer_derivative_over_weight->vector);
    prime->activation = activation_derivative;
    prime->weight = cost_weight_prime;
    prime->error = base_error;

    return;
error:
    return;
}

static float *
optimization_sgd(void *_cell, float learning_rate, float *params)
{
    neural_cell *cell = (neural_cell *)_cell;
    // x - input
    // Eo - previous error
    // m - number of examples
    // W - weight
    // dW - delta for weight update
    // B - bias

    // Eo
    optimization_back_propagation(cell);
    struct neuron_state *body = &cell->context->body;
    struct neuron_state *prime = &cell->context->prime;

    // dW = x * Eo
    vector_values_check(body->weight->vector);
    vector_values_check(prime->weight->vector);
    matrix *delta_weight = Matrix.mul(Matrix.copy(body->weight),
                                     prime->weight);
    // dW = dW * learnin_rate
    Vector.num.mul(delta_weight->vector, learning_rate);
    matrix_check(delta_weight);
    // W = W - dW
    cell->body.weight = Matrix.sub(body->weight,
                                  delta_weight);
    body->weight = cell->body.weight;
    matrix_check(body->weight);

    // B = B - Eo / m
    cell->body.bias -= learning_rate
                      * Vector.sum.all(body->error) / body->error->size;
    body->bias = cell->body.bias;

    // Garbage Conrol
    Matrix.delete(delta_weight);

    return params;

error:
    return NULL;
}
