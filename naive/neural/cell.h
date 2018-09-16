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

typedef struct _neural_cell {
    struct {
        size_t          layer;
        size_t          position;
    }                   coordinates;
    
    neuron              body;
    
    struct _neural_cell **synapse;
    struct _neural_cell **axon;
    
    enum bool           *impulse;
    enum bool           *error;
//    enum bool         *reveres;


} neural_cell;

struct neuron_library {
    neuron               (*create)(neuron_kernel nucleus);
    void                 (*delete)(neuron *n);
    
    neural_cell *        (*fire)(neural_cell *cell, void *signal);
    
    struct {
        neuron *         (*init)(neuron *cell);
        neuron *         (*set)(neuron *cell, matrix *weight, float bias);
    } weight;
    
    struct {
        neuron *         (*functions)(neuron *cell,
                                      struct transfer_library_function transfer,
                                      float (*summation)(vector *transfer),
                                      struct activation_library_function activation,
                                      float (*error)(vector *predicted, vector *target));
        neuron *         (*weight)(neuron *cell, matrix *weight, float bias);
    } set;
    
    
    struct {
        void                 (*impulse)(neural_cell *source, neural_cell *destination);
        matrix *             (*read)(neural_cell *cell);
    } synapse;
};

extern const struct neuron_library Neuron;

#endif /* cell_h */
