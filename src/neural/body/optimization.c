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
    VECTOR_CHECK_PRINT(base_error, "Base error (Eo) vector for back propagate is broken");
    
    // Eo — error from the neurons in front
    size_t axon_dimension = 0;
    for(neural_cell *axon = *cell->axon; axon; axon++) {
        NEURON_CELL_CHECK(axon, "Broken cell in axon terminal");
        
        size_t index = 0;
        for(neural_cell *needle = *axon->synapse; needle; needle++) {
            if(needle == cell) {
                // En = Ei * Wi
                struct neuron_state front_prime = needle->context->prime;
                vector *front_error = front_prime.error;
                VECTOR_CHECK_PRINT(front_error, "Front neuron %zdx%zd error vector is broken", axon->coordinates.layer, axon->coordinates.position);
                front_error = Vector.num.mul(front_error, MATRIX(front_prime.weight, index, 0));
                // Eo += En
                base_error = Vector.add(base_error, front_error);
                axon_dimension++;
            }
            index++;
        }
    }

    if(*cell->axon) {
        // Eo = SUM(En) / Number of cells in axon terminal
        check(axon_dimension, "Axon dimension is 0 when pointer to axon exists");
        base_error = Vector.num.div(base_error, axon_dimension);
    }
    
    // R'(Z)
    vector *activation_derivative = cell->body.nucleus.activation.derivative(cell->context);
    
    // E = Eo * R'(Z) * ... - current layer error
    base_error = Vector.mul(base_error, activation_derivative);
    
    // Z'(X)
    matrix *transfer_derivative_over_signal = cell->body.nucleus.transfer.derivative(cell->context, false);
    
    // Z'(W)? - if last layer, W is 1
    matrix *transfer_derivative_over_weight = *cell->axon
        ? Matrix.seed(Matrix.create(body->weight->rows, body->weight->columns), 1)
        : cell->body.nucleus.transfer.derivative(cell->context, true);
    
    // C'(W) = E * Z'(X)
    matrix *cost_weight_prime = Matrix.copy(body->weight);
    MATRIX_FOREACH(cost_weight_prime) {
        vector *cw_cell_prime = Vector.mul(base_error,
                                   Matrix.column(transfer_derivative_over_signal, row));
        MATRIX(cost_weight_prime, row, column) = Vector.sum.all(cw_cell_prime) / cw_cell_prime->size;
        Vector.delete(prime);
    }
    
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
    
error:
    return;
}

//
//static
//void
//optimization_cell_gradient(neural_cell *cell) {
//    struct neuron_state *body = &cell->context->body;
//    struct neuron_state *prime = &cell->context->prime;
//
//    // Eo — error from the neurons in front
//    vector *total_error = Vector.create(body->signal->rows);
//
//    // Garbage Control
//    if(prime->activation) {
//        Vector.delete(prime->activation);
//        Matrix.delete(prime->signal);
//        Vector.delete(prime->transfer);
//        Vector.delete(prime->weight);
//    }
//
//    // Z'(X)
//    matrix *transfer_derivative_over_signal = cell->body.nucleus.transfer.derivative(cell->context, true);
//    prime->signal = transfer_derivative_over_signal;
//
//    // Z'(W)
//    matrix *transfer_derivative_over_weight = cell->body.nucleus.transfer.derivative(cell->context, false);
//    prime->transfer = Vector.copy(transfer_derivative_over_weight->vector);
//
//    // Eo - Traversal neurons error in front
//    size_t axon_dimension = 0;
//    while(cell->axon[axon_dimension]) {
//        NEURON_CELL_CHECK(cell->axon[axon_dimension], "Broken cell in axon %zd terminal", axon_dimension);
//        neural_cell *axon_cell = cell->axon[axon_dimension];
//        vector *axon_transfer_signal_derivative = Vector.seed(Vector.create(cell->body.signal->rows),
//                                                              1);
//        size_t cell_in_synapse_index = 0;
//        while(axon_cell->synapse[cell_in_synapse_index]) {
//            NEURON_CELL_CHECK(axon_cell->synapse[cell_in_synapse_index], "Broken cell in axon (%zdx%zd) terminal -> %zd synapse terminal", axon_cell->coordinates.layer, axon_cell->coordinates.position, cell_in_synapse_index);
//            if(axon_cell->synapse[cell_in_synapse_index] == cell) {
//                for(size_t sample = 0; sample < axon_transfer_signal_derivative->size; sample++) {
//                    VECTOR(axon_transfer_signal_derivative, sample) = MATRIX(axon_cell->context->prime.signal,
//                                                                             cell_in_synapse_index,
//                                                                             sample);
//                }
//                break;
//            }
//            cell_in_synapse_index++;
//        }
//
//
//        vector *axon_error = Vector.mul(Vector.copy(axon_cell->context->body.error),
//                                        axon_cell->context->prime.activation);
//        axon_error = Vector.mul(axon_error,
//                                axon_transfer_signal_derivative);
//
//        total_error = Vector.add(total_error,
//                                 axon_error);
//
//        Vector.delete(axon_error);
//
//        axon_dimension++;
//    }
//
//    // Eo = C'(O-y) * ...
//    if(axon_dimension == 0) {
//        axon_dimension = 1;
//        total_error = Vector.copy(cell->body.error);
//    }
//    Vector.delete(cell->body.error);
//
//    // R'(Z)
//    vector *activation_derivative = cell->body.nucleus.activation.derivative(cell->context);
//    prime->activation = activation_derivative;
//
//    // En_base = Eo * R'(Z)
//    cell->body.error = Vector.mul(total_error,
//                                  activation_derivative);
//    body->error = cell->body.error;
//
//    // En_weight = En_base * Z'(W) = Eo * R'(Z) * Z'(W)
//    vector **weight_prime = malloc(prime->transfer->size * sizeof(vector *));
//    for (size_t index = 0; index < prime->transfer->size; index++)
//    {
//        float tdo_weight = VECTOR(prime->transfer, index);
//        vector *error_sample = Vector.copy(cell->body.error);
//        weight_prime[index] = Vector.num.mul(error_sample, tdo_weight);
//    }
//    prime->weight = Matrix.transpose(
//                        Matrix.from(
//                            weight_prime,
//                            prime->transfer->size,
//                            cell->body.error->size));
//
//    // Garbage Control
//    for (size_t index = 0; index < prime->transfer->size; index++)
//    {
//        Vector.delete(weight_prime[index]);
//    }
//    free(weight_prime);
//
//    Matrix.print(prime->weight);
//
//error:
//    free(total_error);
//    free(transfer_derivative_over_weight);
//
//    return;
//}
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
//    }

    static float *
    optimization_sgd(void *_cell, float learning_rate, float *params)
    {
        neural_cell *cell = (neural_cell *)_cell;
        struct neuron_state *body = &cell->context->body;
        struct neuron_state *prime = &cell->context->prime;
        
        // x - input
        // Eo - previous error
        // m - number of examples
        // W - weight
        // dW - delta for weight update
        // B - bias

        // Eo
        optimization_back_propagation(cell);

        // dW = x * Eo
        matrix *delta_weight = Matrix.mul(Matrix.copy(body->weight),
                                          prime->weight);
        // dW = dW * learnin_rate
        Vector.num.mul(delta_weight->vector, learning_rate);
  
        // W = W - dW
        cell->body.weight = Matrix.sub(body->weight,
                                       delta_weight);
        body->weight = cell->body.weight;
        
        // B = B - Eo / m
        cell->body.bias -= learning_rate
                           * Vector.sum.all(body->error) / body->error->size;
        body->bias = cell->body.bias;
        
        // Garbage Conrol
        Matrix.delete(delta_weight);

        return params;
}
