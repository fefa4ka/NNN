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

#define NEURON(n_network, layer, position) *((n_network)->neurons + Network.neuron(n_network, layer, position))
#define NEURONS(network, layer) NEURON(network, layer, (size_t)0)

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
    void                 (*delete)(neuron *n);
    
    matrix *             (*fire)(neural_network *network, matrix *signal);
    matrix *             (*error)(neural_network *network, matrix *signal, matrix *target, enum bool derivative);
    size_t               (*neuron)(neural_network *network, size_t layer, size_t position);
    void                 (*train)(neural_network *network, matrix *signal, matrix *target, float learning_rate);
    struct {
        neural_network * (*add)(neural_cell *axon, neural_cell *synapse);
        neural_network * (*remove)(neural_cell *axon, neural_cell *synapse);
    } route;
};

extern const struct network_library Network;

#endif /* network_h */

