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
#define neuron_state_check(neuron, message, ...) { matrix_check_print((neuron).signal, "For neuron signal state." " " message, ##__VA_ARGS__); matrix_check_print((neuron).weight, "Neuron weight state." " " message, ##__VA_ARGS__); vector_check_print((neuron).transfer, "Neuron transfer state." " " message, ##__VA_ARGS__); vector_check_print((neuron).error, "Neuron error state." " " message, ##__VA_ARGS__); }

#define neuron_ccheck(cell, message, ...)                                \
    {                                                                        \
        check((cell), "Out of memory. " message, ##__VA_ARGS__);             \
    }

#define neurons_check(array, message, ...)        \
    check((array), "Out of memory. " message, ##__VA_ARGS__);                                 \
    for (size_t index = 0; array[index]; index++) \
    neuron_ccheck(array[index], "Neuron %zd is broken. " message, index, ##__VA_ARGS__)
#define NEURON(n_network, layer, position) **((n_network)->neurons + Network.get.neuron(n_network, layer, position))

typedef float  (*neuron_summation_function)(matrix *input, matrix *weight, float bias);
typedef float* (*optimization_function)(void *cell, float learning_rate, float* params);

/* Behavior of neuron */
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


/* Neuron in network, with relations with another cells */
typedef struct neural_cell {
    neuron_kernel       nucleus;
    neuron_context *    context;
    
    struct neural_cell  **synapse;
    struct neural_cell  **axon;

    bool           activated;
    bool           dropout;
    bool           *impulse_ready;
    bool           *feedback_ready;
} neural_cell;


/* Neuron library methods */
struct neuron_library {

    neural_cell *        (*create)(neuron_kernel kernel, size_t layer, size_t position);
    void                 (*delete)(neural_cell *cell);

    struct
    {
        neural_cell **   (*layer)(neural_cell *cell);
        neuron_context * (*create)(neural_cell *cell, neural_cell **layer_cells);
        void             (*delete)(neuron_context *context);
    } context;
    
    neural_cell *        (*fire)(neural_cell *cell, matrix *signal);
    neural_cell *    (*activation)(neural_cell *cell);
    
    struct {
        neural_cell *    (*init)(neural_cell *cell);
        neural_cell *    (*set)(neural_cell *cell, matrix *weight, float bias);
    } weight;
    
    struct {
        neural_cell *     (*functions)(neural_cell *cell,
                             struct transfer_library_function transfer,
                             float (*summation)(vector *transfer),
                             struct activation_library_function activation,
                             struct cost_library_function error);
        neural_cell *    (*weight)(neural_cell *cell, matrix *weight, float bias);
        neural_cell *    (*signal)(neural_cell *cell, matrix *data);
    } set;
    
    struct {
        void             (*impulse)(neural_cell *source, neural_cell *destination);
        matrix *         (*read)(neural_cell *cell);
    } synapse;
};

extern const struct neuron_library Neuron;

#endif /* cell_h */
