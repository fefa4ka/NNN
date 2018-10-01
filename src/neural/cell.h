//
//  cell.h
//  naive
//
//  Created by Alexandr Kondratyev on 14/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef cell_h
#define cell_h

#include <stdio.h>

#include <stdio.h>
#include "../math/matrix.h"
#include "body/transfer.h"
#include "body/aggregation.h"
#include "body/activation.h"
#include "body/cost.h"

/* Macros */
#define NEURON_CHECK(neuron, message, ...) { MATRIX_CHECK_PRINT((neuron).signal, "For neuron signal state." " " message, ##__VA_ARGS__); MATRIX_CHECK_PRINT((neuron).weight, "Neuron weight state." " " message, ##__VA_ARGS__); VECTOR_CHECK_PRINT((neuron).transfer, "Neuron transfer state." " " message, ##__VA_ARGS__); VECTOR_CHECK_PRINT((neuron).error, "Neuron error state." " " message, ##__VA_ARGS__); }

#define NEURON_CELL_IS_SYNC(cell) ((cell->context != NULL) && \
                                   (cell->context->body.weight == cell->body.weight \
                                    && cell->context->body.bias == cell->body.bias \
                                    && cell->context->body.signal == cell->body.signal \
                                    && cell->context->body.transfer == cell->body.transfer \
                                    && cell->context->body.activation == cell->body.axon \
                                    && cell->context->body.error == cell->body.error) \
                                  || cell->context == NULL)
#define NEURON_CELL_CHECK(cell, message, ...)                                \
    {                                                                        \
        check((cell), "Out of memory. " message, ##__VA_ARGS__);             \
        check(NEURON_CELL_IS_SYNC((cell)), "Cell context is't synchronized." \
                                           " " message,                      \
              ##__VA_ARGS__);                                                \
        NEURON_CHECK((cell)->body, message, ##__VA_ARGS__);                  \
    }
#define NEURONS_CHECK(array, message, ...)        \
    check((array), "Out of memory. " message, ##__VA_ARGS__);                                 \
    for (size_t index = 0; array[index]; index++) \
    NEURON_CELL_CHECK(array[index], "Neuron %zd is broken. " message, index, ##__VA_ARGS__)
#define NEURON(n_network, layer, position) *((n_network)->neurons + Network.neuron(n_network, layer, position))


typedef float  (*neuron_summation_function)(matrix *input, matrix *weight, float bias);
typedef float* (*optimization_function)(void *cell, float learning_rate, float* params);


typedef struct neuron_kernel {
    struct
    transfer_library_function      transfer;
    
    float                          (*summation)(vector *transfer);
    
    struct
    activation_library_function    activation;
    
    struct
    cost_library_function          error;
    optimization_function          optimization;
} neuron_kernel;

typedef struct {
    matrix *                     signal;
    matrix *                     weight;
    float                        bias;

    neuron_kernel                nucleus;

    vector *                     transfer;
    vector *                     axon;
    vector *                     error;
} neuron;

typedef struct neural_cell {
    struct {
        size_t          layer;
        size_t          position;
    }                   coordinates;
    
    neuron              body;
    neuron_context *    context;
    
    struct neural_cell  **synapse;
    struct neural_cell  **axon;
    
    enum bool           *impulse;
    enum bool           *reverse;
} neural_cell;

struct neuron_library {
    neuron               (*create)(neuron_kernel nucleus);
    void                 (*delete)(neuron *n);
    
    struct {
        neural_cell     (*create)(neuron_kernel kernel, size_t layer, size_t position);
        void            (*delete)(neural_cell *cell);
    } cell;

    struct
    {
        neural_cell **   (*layer)(neural_cell *cell);
        neuron_context * (*create)(neural_cell *cell);
        void             (*delete)(neuron_context *context);
    } context;
    
    neural_cell *        (*fire)(neural_cell *cell, matrix *signal);
    
    struct {
        neuron *         (*init)(neuron *cell);
        neuron *         (*set)(neuron *cell, matrix *weight, float bias);
    } weight;
    
    struct {
        neuron *(*functions)(neuron *cell,
                             struct transfer_library_function transfer,
                             float (*summation)(vector *transfer),
                             struct activation_library_function activation,
                             struct cost_library_function error);
        neuron *(*weight)(neuron *cell, matrix *weight, float bias);
    } set;
    
    
    struct {
        void             (*impulse)(neural_cell *source, neural_cell *destination);
        matrix *         (*read)(neural_cell *cell);
    } synapse;
};

extern const struct neuron_library Neuron;

#endif /* cell_h */
