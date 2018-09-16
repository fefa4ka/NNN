//
//  cell.c
//  naive
//
//  Created by Alexandr Kondratyev on 14/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "cell.h"

static neuron               neuron_create(neuron_kernel nucleus);
static void                 neuron_delete(neuron *body);
static neural_cell *        neuron_fire(neural_cell *cell, void *signal);
static void                 neuron_fire_forward(neural_cell *cell);
static void                 neuron_impulse(neural_cell *source, neural_cell *destination);

static matrix *             neuron_collect_synapse_signal(neural_cell *cell);

static neuron *             neuron_init_weight(neuron *cell);
static neuron *             neuron_set_weight(neuron *cell, matrix *weight, double bias);

static neuron *             neuron_set_functions(neuron *cell,
                                                 struct transfer_library_function transfer,
                                                 double (*summation)(vector *transfer),
                                                 struct activation_library_function activation,
                                                 double (*error)(vector *predicted, vector *target));


/* Library Structure */
const struct neuron_library Neuron = {
    .create = neuron_create,
    .delete = neuron_delete,
    
    .weight = {
        .init = neuron_init_weight,
        .set = neuron_set_weight
    },
    
    .set = {
        .functions = neuron_set_functions
    },
    
    .fire = neuron_fire,
    
    .synapse = {
        .read = neuron_collect_synapse_signal,
        .impulse = neuron_impulse
    }
};

/* Neuron Life Cycle */
static
neuron
neuron_create(neuron_kernel nucleus) {
    double bias = random_range(-1, 1);
    
    return (neuron){
        .signal = Matrix.create(1, 1),
        .weight = Matrix.seed(Matrix.create(1, nucleus.transfer.dimension), 0),
        .bias = bias,
        .nucleus = nucleus,
        .transfer = Vector.create(1),
        .axon = Vector.create(1),
        .error = Vector.create(1)
    };
}

static
void
neuron_delete(neuron *body) {
    Matrix.delete(body->signal);
    Matrix.delete(body->weight);
    
    Vector.delete(body->transfer);
    Vector.delete(body->axon);
    Vector.delete(body->error);
}

/* Neuron Firing */

static
neural_cell *
neuron_fire(neural_cell *cell, void *data) {
    neuron *body = &cell->body;
    neuron_kernel *kernel = &body->nucleus;
    
    Vector.delete(body->transfer);
    Vector.delete(body->axon);
    Matrix.delete(body->signal);
    
    if(IS(data, VECTOR_TYPE)) {
        vector *signal = Vector.copy((vector*)data);
        memset(cell->impulse, 0, signal->size * sizeof(enum bool));

        body->signal = Matrix.from(signal, signal->size, 1);
        neuron_init_weight(&cell->body);

        body->axon = Vector.create(1);
        body->transfer = Vector.create(1);
        VECTOR(body->transfer, 0) = kernel->transfer.signal(signal,
                                                            body->weight,
                                                            body->bias);
        VECTOR(body->axon, 0) = kernel->activation.of(VECTOR(body->transfer, 0));

        Vector.delete(signal);
    }

    if(IS(data, MATRIX_TYPE)) {

        body->signal = Matrix.copy((matrix*)data);
        free(cell->impulse);
        cell->impulse = calloc(body->signal->columns, sizeof(enum bool));
//        memset(cell->impulse, 0, body->signal->columns * sizeof(enum bool));

        neuron_init_weight(&cell->body);
        
        body->transfer = kernel->transfer.samples(body->signal,
                                                  body->weight,
                                                  body->bias);
        body->axon = Vector.map(Vector.copy(body->transfer),
                                kernel->activation.of);
        
        

    }

    neuron_fire_forward(cell);
    memset(cell->impulse, 0, body->signal->columns * sizeof(enum bool));
    return cell;
}

// Fire forward from source neuron to axon terminals
static
void
neuron_fire_forward(neural_cell *cell) {
    size_t index = 0;
    
    while (cell->axon[index]) {
        neuron_impulse(cell, cell->axon[index++]);
    }
}

// Send impulse from source neuron to destination neuron
static
void
neuron_impulse(neural_cell *source, neural_cell *destination) {
    size_t index = 0;
    enum bool fire = true;
    enum bool impulse = false;
    
    while(destination->synapse[index]) {
        if(destination->synapse[index] == source) {
            destination->impulse[index] = true;
            impulse = true;
        }
        
        if(destination->impulse[index] == false) {
            fire = false;
        }
        
        if(impulse && fire == false){
            return;
        }
        
        index++;
    }
    
    if(fire) {
        matrix *signal = neuron_collect_synapse_signal(destination);
        Neuron.fire(destination,
                    signal);
        
        Matrix.delete(signal);
    }
}



// Neuron input signal
static
matrix *
neuron_collect_synapse_signal(neural_cell *cell) {
    size_t dimension = 0;
    
    vector **impulse = malloc(cell->body.signal->columns * sizeof(vector *));
    while(cell->synapse[dimension]) {
        if(dimension > cell->body.signal->columns) {
            impulse = realloc(impulse, (dimension + 1) * sizeof(vector *));
        }
        
        impulse[dimension] = cell->synapse[dimension]->body.axon;

        dimension++;
    }

    size_t bunch_size = impulse[0]->size;
    matrix *signal = Matrix.transpose(Matrix.from(impulse, dimension, bunch_size));

    free(impulse);
    
    return signal;
}


/* Kernel */
static
neuron *
neuron_set_functions(neuron *cell,
                     struct transfer_library_function transfer,
                     double (*summation)(vector *transfer),
                     struct activation_library_function activation,
                     double (*error)(vector *predicted, vector *target)) {
    cell->nucleus.summation = summation;
    cell->nucleus.activation = activation;
    
    return cell;
}

/* Weight */
static
neuron *
neuron_init_weight(neuron *body) {
    size_t dimension_diff = body->signal->columns - body->weight->rows;
    
    if(dimension_diff) {
        Matrix.reshape(body->weight, body->signal->columns, body->weight->columns);
        if(dimension_diff > 0) {
            matrix *weight = Matrix.seed(Matrix.create(dimension_diff, body->weight->columns),
                                         0);
            MATRIX_FOREACH(weight) {
                MATRIX(body->weight, body->weight->rows - row - 1, body->weight->columns - column - 1) = MATRIX(weight, row, column);
            }
            
            Matrix.delete(weight);
        }
    }
    
    return body;
}

static
neuron *
neuron_set_weight(neuron *cell, matrix *weight, double bias) {
    cell->weight = weight;
    cell->bias = bias;
    
    return cell;
}

