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
static neural_cell *        neuron_fire(neural_cell *cell, matrix *signal);
static void                 neuron_fire_forward(neural_cell *cell);
static void                 neuron_impulse(neural_cell *source, neural_cell *destination);

static neuron_context *     neuron_get_context(neural_cell *cell);
static neural_cell **       neuron_get_cell_layer(neural_cell *cell);

static matrix *             neuron_collect_synapse_signal(neural_cell *cell);

static neuron *             neuron_init_weight(neuron *cell);
static neuron *             neuron_set_weight(neuron *cell, matrix *weight, float bias);

static neuron *             neuron_set_functions(neuron *cell,
                                                 struct transfer_library_function transfer,
                                                 float (*summation)(vector *transfer),
                                                 struct activation_library_function activation,
                                                 float (*error)(vector *predicted, vector *target));


/* Library Structure */
const struct neuron_library Neuron = {
    .create = neuron_create,
    .delete = neuron_delete,
    
    .context = neuron_get_context,
    
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
    float bias = random_range(-1, 1);
    
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

/* Neuron context */
static
neural_cell **
neuron_get_cell_layer(neural_cell *cell) {
    neural_cell **layer_cells = calloc(1, sizeof(neural_cell*));
    size_t number_of_cells_in_layer = 0;
    
    check_memory(layer_cells);
    NEURON_CELL_CHECK(cell, "Argument cell");
    
    for(size_t synapse_index = 0; cell->synapse[synapse_index]; synapse_index++) {
        neural_cell *input_cell = cell->synapse[synapse_index];
        NEURON_CELL_CHECK(input_cell, "(%zdx%zd) Input cell SI = %zd", cell->coordinates.layer, cell->coordinates.position, synapse_index);
        
        for(size_t input_axon_index = 0; input_cell->axon[input_axon_index]; input_axon_index++) {
            neural_cell *axon_cell = input_cell->axon[input_axon_index];
            NEURON_CELL_CHECK(axon_cell, "(%zdx%zd) Axon cell SI = %zd, IAI = %zd", input_cell->coordinates.layer, input_cell->coordinates.position, synapse_index, input_axon_index);
            
            size_t layer_cell_index = 0;
            while(layer_cells[layer_cell_index]) {
                neural_cell *layer_cell = layer_cells[layer_cell_index];
                NEURON_CELL_CHECK(layer_cell, "(%zdx%zd) Layer cell. SI = %zd, IAI = %zd, LCI = %zd", axon_cell->coordinates.layer, axon_cell->coordinates.position, synapse_index, input_axon_index, layer_cell_index);
                
                if(layer_cell == axon_cell) {
                    break;
                }
                layer_cell_index++;
            }
            
            if(layer_cell_index == number_of_cells_in_layer) {
                layer_cells = realloc(layer_cells, (number_of_cells_in_layer + 1) * sizeof(neural_cell*));
                check_memory(layer_cells);
                layer_cells[number_of_cells_in_layer] = axon_cell;
                NEURON_CELL_CHECK(layer_cells[number_of_cells_in_layer], "Uniq layer cell %zd", number_of_cells_in_layer);
                layer_cells[number_of_cells_in_layer + 1] = NULL;
                
                number_of_cells_in_layer++;
            }
        }
    }
    
    check(layer_cells[number_of_cells_in_layer] == NULL, "Last element not NULL");
    
    return layer_cells;
    
error:
    free(layer_cells);
    return NULL;
}

static
neuron_context *
neuron_get_context(neural_cell *cell) {
    NEURON_CELL_CHECK(cell, "On get context")
    neural_cell **layer_neurons = neuron_get_cell_layer(cell);
    check_memory(layer_neurons);
    
    size_t layer_index = 0;
    vector **layer_axon = malloc(sizeof(vector*));
    vector **layer_error = malloc(sizeof(vector*));
    
    while(layer_neurons[layer_index]) {
        size_t layer_size = (layer_index + 1) * sizeof(vector*);
        
        layer_axon = realloc(layer_axon, layer_size);
        check_memory(layer_axon);
        layer_axon[layer_index] = layer_neurons[layer_index]->body.axon;
        check_memory(layer_axon[layer_index]);
        VECTOR_CHECK_PRINT(layer_axon[layer_index],
                           "[%zd] Axon not vector", layer_index);
        
        layer_error = realloc(layer_axon, layer_size);
        check_memory(layer_error);
        layer_error[layer_index] = layer_neurons[layer_index]->body.error;
        VECTOR_CHECK_PRINT(layer_error[layer_index],
                           "[%zd] Error not vector", layer_index);
        
        layer_index++;
    }
    
    free(layer_neurons);
    
    neuron_context *context = malloc(sizeof(neuron_context));
    struct neuron_state state = {
        .weight     = cell->body.weight,
        .bias       = cell->body.bias,
        .signal     = cell->body.signal,
        .transfer   = cell->body.transfer,
        .activation = cell->body.axon,
        .error      = cell->body.error
    };
    struct layer_state layer = {
        .dimension = layer_index,
        .axon = layer_axon,
        .error = layer_error
    };
    context->body = state;
    context->layer = layer;
    
    return context;
    
error:
    return NULL;
}


/* Neuron Firing */
static
neural_cell *
neuron_fire(neural_cell *cell, matrix *data) {
    NEURON_CELL_CHECK(cell, "Argument Cell");
    MATRIX_CHECK_PRINT(data, "For neuron fire");
    
    neuron *body = &cell->body;
    neuron_kernel *kernel = &body->nucleus;
    
    // Clear previos state
    memset(cell->impulse, 0, body->signal->columns * sizeof(enum bool));
    
    Matrix.delete(body->signal);
    body->signal = Matrix.copy((matrix*)data);
    MATRIX_CHECK_PRINT(body->signal, "Broken fire signal");
    cell->context->body.signal = body->signal;
    
    neuron_init_weight(&cell->body);
    
    Vector.delete(body->transfer);
    body->transfer = kernel->transfer.function(body->signal,
                                               body->weight,
                                               body->bias);
    VECTOR_CHECK_PRINT(body->transfer, "Broken fire transfer");
    cell->context->body.transfer = body->transfer;
    
    Vector.delete(body->axon);
    body->axon = kernel->activation.of(cell->context);
    VECTOR_CHECK_PRINT(body->transfer, "Broken fire axon");
    cell->context->body.activation = body->axon;
    
    neuron_fire_forward(cell);
    
    return cell;

error:
    return NULL;
}

// Fire forward from source neuron to axon terminals
static
void
neuron_fire_forward(neural_cell *cell) {
    NEURON_CELL_CHECK(cell, "Broken cell for fire forward");
    
    size_t index = 0;
    
    while (cell->axon[index]) {
        neuron_impulse(cell, cell->axon[index++]);
    }
    
error:
    return;
}

// Send impulse from source neuron to destination neuron
static
void
neuron_impulse(neural_cell *source, neural_cell *destination) {
    NEURON_CELL_CHECK(source, "Broken source cell");
    NEURON_CELL_CHECK(destination, "Broken destination cell");
    
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

error:
    return;
}



// Neuron input signal
static
matrix *
neuron_collect_synapse_signal(neural_cell *cell) {
    size_t dimension = 0;
    vector **impulse;
    NEURON_CELL_CHECK(cell, "Broken cell from argument");
    
    impulse = malloc(cell->body.signal->columns * sizeof(vector *));
    while(cell->synapse[dimension]) {
        if(dimension > cell->body.signal->columns) {
            impulse = realloc(impulse, (dimension + 1) * sizeof(vector *));
            check_memory(impulse);
        }
        
        impulse[dimension] = cell->synapse[dimension]->body.axon;
        dimension++;
        
        VECTOR_CHECK(impulse[dimension - 1]);
    }

    size_t bunch_size = impulse[0]->size;
    matrix *signal = Matrix.transpose(Matrix.from(impulse, dimension, bunch_size));
    MATRIX_CHECK_PRINT(signal, "From bunch of vectors");
    
    free(impulse);
    
    return signal;
    
error:
    if(dimension > 0) { free(impulse); }
    return NULL;
}


/* Kernel */
static
neuron *
neuron_set_functions(neuron *cell,
                     struct transfer_library_function transfer,
                     float (*summation)(vector *transfer),
                     struct activation_library_function activation,
                     float (*error)(vector *predicted, vector *target)) {
    cell->nucleus.summation = summation;
    cell->nucleus.activation = activation;
    
    return cell;
}

/* Weight */
static
neuron *
neuron_init_weight(neuron *body) {
    NEURON_CHECK(*body, "Init weight");
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

error:
    return NULL;
}

static
neuron *
neuron_set_weight(neuron *cell, matrix *weight, float bias) {
    cell->weight = weight;
    cell->bias = bias;
    
    return cell;
}

