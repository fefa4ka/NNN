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

// https://ml-cheatsheet.readthedocs.io/en/latest/backpropagation.html
static
void
optimization_back_propagation(neural_cell *cell) {
    struct neuron_state *body = &cell->context->body;
    struct neuron_state *prime = &cell->context->prime;
    
    // Let's begin backpropagating the error derivatives.
    // Eo = C'(O-y) * ... if output layer
    vector *base_error = *cell->axon
                            ? Vector.create(body->signal->rows)
                            : Vector.copy(cell->context->prime.error);
    vector_check_print(base_error, "Base error (Eo) vector for back propagate is broken");
    vector_values_check(base_error);

    // Eo — error from the neurons in front
    size_t axon_dimension = 0;
    for(neural_cell *axon = cell->axon[0]; cell->axon[axon_dimension]; axon = cell->axon[axon_dimension]) {
        neuron_ccheck(axon, "Broken cell in axon terminal");
        size_t signal_dimension = axon->context->body.signal->columns;
        
        for(size_t index = 0; index < signal_dimension; index++) {
            if(axon->synapse[index] == cell) {
                // En = Ei * Wi
                struct neuron_state front_prime = axon->context->prime;
                vector *front_error = Vector.copy(front_prime.error);
                vector_values_check(front_error);
                vector_check_print(front_error, "Error vector of front neuron %zdx%zd", axon->context->layer_index, axon->context->position);
                // dE/dY = SUM(dX/dY * dE/dX)
                front_error = Vector.num.mul(front_error, VECTOR(front_prime.transfer, index));
                // Eo += En
                vector_values_check(base_error);
                base_error = Vector.add(base_error, front_error);
                vector_values_check(base_error);
                axon_dimension++;

                free(front_error);
                break;
            }
        }
    }

    // if(*cell->axon) {
    //     // Eo = SUM(En) / Number of cells in axon terminal
    //     check(axon_dimension, "Axon dimension is 0 when pointer to axon exists");
    //     base_error = Vector.num.div(base_error, axon_dimension);
    //     vector_values_check(base_error);
    // }

    vector_check(base_error);
    
    // Additional derivatives
    // To help compute dEo/dW, we additionaly store for each node
    // two more derivatives: ho the error changes with:
    //      * the total input of node dEo/dX
    //      * the output of the node dEo/dZ

    // For each input to neuron let us calculate the derivative with respect to each weight. 
    // Z - transfer function
    // Z'(W)
    matrix *transfer_derivative_over_signal = cell->nucleus.transfer.derivative(cell->context, true);
    matrix_check(transfer_derivative_over_signal);

    
    // Z'(X)? - if last layer, W is 1
    matrix *transfer_derivative_over_weight = cell->nucleus.transfer.derivative(cell->context, false);
      //  : Matrix.seed(Matrix.create(body->weight->rows, body->weight->columns), 1);

    // As soon as we have the error derivative with respect 
    // to the total input of a node, we can get the error derivative 
    // with respect to the weights coming into that node.
    // E = Eo * Z'(W) * ...
    // vector_foreach(base_error) {
    //     float *sample_error = &VECTOR(base_error, index);
    //     vector *sample_error_signal = Vector.num.mul(Vector.copy(transfer_derivative_over_signal->vector), *sample_error);

    //     VECTOR(base_error, index) = Vector.sum.all(sample_error_signal) / sample_error_signal->size;
    //     Vector.delete(sample_error_signal);
    // }
    
    // R - activation function
    // R'(Z)
    // Calculate the derivative of each output with respect to their input.
    vector *activation_derivative = cell->nucleus.activation.derivative(cell->context);
    vector_check(activation_derivative);
    vector_values_check(base_error);

    // E = Eo * R'(Z) * ... - current layer error
    base_error = Vector.mul(base_error, activation_derivative);
    vector_values_check(base_error);
   
    // Cost = Cost(Activation(Transfer(XW))) = C(R(Z(XW)))
    // C'(W) = C'(R) * R'(Z) * Z'(W) = E * Z'(X)
    matrix *cost_weight_prime = Matrix.copy(body->weight);
    vector_values_check(transfer_derivative_over_signal->vector);
    matrix_foreach(cost_weight_prime) {
        // float term = MATRIX(transfer_derivative_over_signal, row, column);
        vector *cw_prime = Matrix.column(transfer_derivative_over_signal, row);
        cw_prime = Vector.mul(cw_prime, base_error);

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

    Matrix.delete(transfer_derivative_over_weight);

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
    // matrix *delta_weight = Matrix.mul(Matrix.copy(body->weight),
    //                                   prime->weight);
    matrix *delta_weight = Matrix.copy(prime->weight);

    // dW = dW * learnin_rate
    Vector.num.mul(delta_weight->vector, learning_rate);
    
    matrix_check(delta_weight);


    // W = W - dW
    body->weight = Matrix.sub(body->weight,
                              delta_weight);
    matrix_check(body->weight);

    // B = B - Eo / m
    // body->bias -= learning_rate
    //                  * Vector.sum.all(prime->error) / prime->error->size;

    // Garbage Conrol
    Matrix.delete(delta_weight);

    return params;

error:
    return NULL;
}
