//
//  network.h
//  math
//
//  Created by Alexandr Kondratyev on 10/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef network_h
#define network_h

#include <stdio.h>
//#include <omp.h>
#include "cell.h"
#include "body/optimization.h"
#include "../data/set.h"

#define NEURONS(network, layer) NEURON(network, layer, (size_t)0)
#define network_check(network)                                                                                                               \
    for (size_t index = 0; index < network->resolution.size; index++)                                                                        \
    {                                                                                                                                        \
        neural_cell *cell = network->neurons[index];                                                                                        \
        neuron_ccheck(cell, "Neuron cell %zd is broken", index);                                                                         \
        neurons_check(cell->synapse, "Neuron cell %zdx%zd synapse terminal is broken", cell->context->layer_index, cell->context->position); \
        neurons_check(cell->axon, "Neuron cell %zdx%zd axon terminal is broken", cell->context->layer_index, cell->context->position);       \
    }

/* Training history */
typedef struct {
    float               error;
    float               accuracy; 
} network_metrics;

typedef struct {
   network_metrics      train;
   network_metrics      validation;
   network_metrics      test;
} network_loss;


/* Neural network with defined shape and list of neurons */
typedef struct {
    struct {
        size_t*   dimensions;
        size_t    layers;
        size_t    size;
    }             resolution;
    
    neural_cell   **neurons;
    
    network_loss  *history;
} neural_network;

/* Definition of layer */
typedef struct {
    neuron_kernel       kernel;
    
    void                (*router)(neural_network *network, size_t layer);
    
    size_t              dimension;
    float               dropout;
} neural_layer;


/* Library methods */
struct network_library {
    neural_network       (*create)(neural_layer layers[]);
    void                 (*delete)(neural_network *network);
    
    matrix *             (*fire)(neural_network *network, matrix *signal);
    void                 (*train)(neural_network *network, data_batch *training_data, float learning_rate, int epoch);
    float                (*error)(neural_network *network, matrix *signal, matrix *target);
    
    struct {
        size_t           (*neuron)(neural_network *network, size_t layer, size_t position);
        neural_cell **   (*layer)(neural_network *network, size_t layer);
    } get;

    struct {
        neural_network * (*add)(neural_cell *axon, neural_cell *synapse);
        neural_network * (*remove)(neural_cell *axon, neural_cell *synapse);
    } route;
};

extern const struct network_library Network;

#endif /* network_h */

