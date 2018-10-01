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
#include "cell.h"
#include "body/optimization.h"
#include "../data/set.h"

#define NEURONS(network, layer) NEURON(network, layer, (size_t)0)
#define NETWORK_CHECK(network)                                                                                                               \
    for (size_t index = 0; index < network->resolution.size; index++)                                                                        \
    {                                                                                                                                        \
        neural_cell *cell = &network->neurons[index];                                                                                        \
        NEURON_CELL_CHECK(cell, "Neuron cell %zd is broken", index);                                                                         \
        NEURONS_CHECK(cell->synapse, "Neuron cell %zdx%zd synapse terminal is broken", cell->coordinates.layer, cell->coordinates.position); \
        NEURONS_CHECK(cell->axon, "Neuron cell %zdx%zd axon terminal is broken", cell->coordinates.layer, cell->coordinates.position);       \
    }

typedef struct {
    struct {
        size_t*   dimensions;
        size_t    layers;
        size_t    size;
    }             resolution;
    
    neural_cell   *neurons;
} neural_network;

typedef struct {
    neuron_kernel       kernel;
    
    void                (*router)(neural_network *network, size_t layer);
    
    size_t              dimension;
} neural_layer;


struct network_library {
    neural_network       (*create)(neural_layer layers[]);
    void                 (*delete)(neural_network *network);
    
    matrix *             (*fire)(neural_network *network, matrix *signal);
    matrix *             (*error)(neural_network *network, matrix *signal, matrix *target, enum bool derivative);
    size_t               (*neuron)(neural_network *network, size_t layer, size_t position);
    void                 (*train)(neural_network *network, data_batch *training_data, float learning_rate, int epoch);
    struct {
        neural_network * (*add)(neural_cell *axon, neural_cell *synapse);
        neural_network * (*remove)(neural_cell *axon, neural_cell *synapse);
    } route;
};

extern const struct network_library Network;

#endif /* network_h */

