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

static neural_cell          neuron_cell_create(neuron_kernel kernel, size_t layer, size_t position);
static void                 neuron_cell_delete(neural_cell *cell);

static neural_cell *        neuron_fire(neural_cell *cell, matrix *signal);
static void                 neuron_fire_forward(neural_cell *cell);
static void                 neuron_impulse(neural_cell *source, neural_cell *destination);

static neuron_context *     neuron_context_create(neural_cell *cell);
static void                 neuron_context_delete(neuron_context *context);
static neural_cell **       neuron_get_cell_layer(neural_cell *cell);

static matrix *             neuron_collect_synapse_signal(neural_cell *cell);

static neuron *             neuron_init_weight(neuron *cell);
static neuron *             neuron_set_functions(neuron *cell,
                                                 struct transfer_library_function transfer,
                                                 float (*summation)(vector *transfer),
                                                 struct activation_library_function activation,
                                                 struct cost_library_function error);
static neuron *neuron_set_weight(neuron *cell, matrix *weight, float bias);

/* Library Structure */
const struct neuron_library Neuron = {
    .create = neuron_create,
    .delete = neuron_delete,

    .cell = {
        .create = neuron_cell_create,
        .delete = neuron_cell_delete
    },
    
    .context = {
        .layer = neuron_get_cell_layer,
        .create = neuron_context_create,
        .delete = neuron_context_delete
    },
    
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
    
    return (neuron) {
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

/* Cell */
static
neural_cell 
neuron_cell_create(neuron_kernel kernel, size_t layer, size_t position) {
    return (neural_cell) {
            .body = Neuron.create(kernel),
            .coordinates = {
                .layer = layer,
                .position = position
            },
            .axon = calloc(1, sizeof(neural_cell *)),
            .synapse = calloc(1, sizeof(neural_cell *)),
            .impulse = calloc(1, sizeof(enum bool)),
            .reverse = calloc(1, sizeof(enum bool))
        };
}

static
void
neuron_cell_delete(neural_cell *cell) {
    free(cell->axon);
    free(cell->synapse);
    free(cell->impulse);
    free(cell->reverse);

    Neuron.delete(&cell->body);

    *cell = (neural_cell){0};
}

/* Neuron context */
static
neural_cell **
neuron_get_cell_layer(neural_cell *cell) {
    neural_cell **layer_cells = calloc(1, sizeof(neural_cell*));
    size_t number_of_cells_in_layer = 0;

    check_memory(layer_cells);
    NEURON_CELL_CHECK(cell, "Argument cell");

    neural_cell **terminal = cell->synapse;
    enum bool is_forward = false;
    if (*terminal == NULL){
        terminal = cell->axon;
        is_forward = true;
    }
    NEURONS_CHECK(terminal, "Terminal array of cell %zdx%zd is broken", cell->coordinates.layer, cell->coordinates.position);
    for(size_t terminal_index = 0; terminal[terminal_index]; terminal_index++) {
        neural_cell *input_cell = terminal[terminal_index];
        NEURON_CELL_CHECK(input_cell, "(%zdx%zd) Input cell TI = %zd", cell->coordinates.layer, cell->coordinates.position, terminal_index);

        neural_cell **input_cell_terminal = is_forward
            ? input_cell->synapse 
            : input_cell->axon;

        NEURONS_CHECK(input_cell_terminal, "Terminal array of input cell %zdx%zd is broken", input_cell->coordinates.layer, input_cell->coordinates.position);
        for (size_t input_terminal_index = 0; input_cell_terminal[input_terminal_index]; input_terminal_index++)
        {
            neural_cell *axon_cell = input_cell_terminal[input_terminal_index];
            NEURON_CELL_CHECK(axon_cell, "(%zdx%zd) Terminal cell TI = %zd, ITI = %zd", input_cell->coordinates.layer, input_cell->coordinates.position, terminal_index, input_terminal_index);
            
            size_t layer_cell_index = 0;
            NEURONS_CHECK(layer_cells, "Uniq array of cells is broken");
            while(layer_cells[layer_cell_index]) {
                neural_cell *layer_cell = layer_cells[layer_cell_index];
                NEURON_CELL_CHECK(layer_cell, "(%zdx%zd) Layer cell. TI = %zd, ITI = %zd, LCI = %zd", axon_cell->coordinates.layer, axon_cell->coordinates.position, terminal_index, input_terminal_index, layer_cell_index);
                
                if(layer_cell == axon_cell) {
                    break;
                }
                layer_cell_index++;
            }
            
            if(layer_cell_index == number_of_cells_in_layer) {
                layer_cells = realloc(layer_cells, (number_of_cells_in_layer + 2) * sizeof(neural_cell*));
                check_memory(layer_cells);
                layer_cells[number_of_cells_in_layer] = axon_cell;
                NEURON_CELL_CHECK(layer_cells[number_of_cells_in_layer], "Uniq layer cell %zd", number_of_cells_in_layer);

                layer_cells[++number_of_cells_in_layer] = NULL;                
            }
        }
    }
    
    check(layer_cells[number_of_cells_in_layer] == NULL, "Last element isn't NULL");
    return layer_cells;
    
error:
    free(layer_cells);
    return NULL;
}

static
neuron_context *
neuron_context_create(neural_cell *cell) {
    NEURON_CELL_CHECK(cell, "On get context")
    neural_cell **layer_neurons = neuron_get_cell_layer(cell);
    check_memory(layer_neurons);
    NEURONS_CHECK(layer_neurons, "Layer for %zdx%zd cell building failed.", cell->coordinates.layer, cell->coordinates.position);

    size_t layer_index = 0;
    vector ***layer_axon = malloc(sizeof(vector**));
    vector ***layer_error = malloc(sizeof(vector**));
    
    while(layer_neurons[layer_index]) {
        NEURON_CELL_CHECK(layer_neurons[layer_index], "Cell %zd from layer is broken", layer_index);
        size_t layer_size = (layer_index + 2) * sizeof(vector**);
        
        layer_axon = realloc(layer_axon, layer_size);
        check_memory(layer_axon);
        layer_axon[layer_index] = &(layer_neurons[layer_index]->body.axon);
        check_memory(layer_axon[layer_index]);
        VECTOR_CHECK_PRINT(*layer_axon[layer_index],
                           "[%zd] Axon vector is broken", layer_index);
        
        layer_error = realloc(layer_error, layer_size);
        check_memory(layer_error);
        layer_error[layer_index] = &(layer_neurons[layer_index]->body.error);
        VECTOR_CHECK_PRINT(*layer_error[layer_index],
                           "[%zd] Error vector is broken", layer_index);
        
        layer_index++;
        layer_axon[layer_index] = NULL;
        layer_error[layer_index] = NULL;
    }

    free(layer_neurons);
    
    neuron_context *context = malloc(sizeof(neuron_context));
    context->prime = (struct neuron_state) {0};
    context->body = (struct neuron_state) {
        .weight     = cell->body.weight,
        .bias       = cell->body.bias,
        .signal     = cell->body.signal,
        .transfer   = cell->body.transfer,
        .activation = cell->body.axon,
        .error      = cell->body.error
    };
    context->layer = (struct layer_state) {
        .dimension = layer_index,
        .axon = layer_axon,
        .error = layer_error
    };

    return context;
    
error:
    return NULL;
}

static
void
neuron_context_delete(neuron_context *context) {
    free(context->layer.axon);
    free(context->layer.error);
    free(context);
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

    vector *activation = kernel->activation.of(cell->context);
    VECTOR_CHECK_PRINT(activation, "Broken fire activation");
    Vector.delete(body->axon);
    
    cell->context->body.activation = body->axon = activation;

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
        NEURON_CELL_CHECK(cell->synapse[dimension], "Broken cell in synapse terminal");

        if (dimension > cell->body.signal->columns) {
            impulse = realloc(impulse, (dimension + 1) * sizeof(vector *));
            check_memory(impulse);
        }
        
        impulse[dimension] = cell->synapse[dimension]->body.axon;
        VECTOR_CHECK(impulse[dimension]);
        
        dimension++;
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
                     struct cost_library_function error) {
    cell->nucleus.transfer = transfer;
    cell->nucleus.summation = summation;
    cell->nucleus.activation = activation;
    cell->nucleus.error = error;

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

        if (dimension_diff > 0) {
            matrix *weight = Matrix.seed(Matrix.create(dimension_diff, body->weight->columns),
                                         0);
            MATRIX_FOREACH(weight) {
                MATRIX(body->weight, body->weight->rows - row - 1, body->weight->columns - column - 1) = MATRIX(weight, row, column);
            }
            
            Matrix.delete(weight);
        }
    }

    MATRIX_CHECK(body->weight);

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

