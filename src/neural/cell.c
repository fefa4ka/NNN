//
//  cell.c
//  naive
//
//  Created by Alexandr Kondratyev on 14/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "cell.h"

static neural_cell *        cell_create(neuron_kernel neuron_kernel, size_t layer, size_t position);
static void                 cell_delete(neural_cell *cell);

static neural_cell *        fire(neural_cell *cell, matrix *signal);
static neural_cell *        transfer(neural_cell *cell);
static neural_cell *        activation(neural_cell *cell);
static void                 fire_forward(neural_cell *cell);
static void                 impulse(neural_cell *source, neural_cell *destination);

static neural_cell **       get_cell_layer(neural_cell *cell);
//static struct layer_state   context_layer(neural_cell *cell, neural_cell **layer_cells);

static neuron_context *     context_create(neural_cell *cell, neural_cell **layer_cells);
static void                 context_delete(neuron_context *context);

static matrix *             collect_synapse_signal(neural_cell *cell);

static neural_cell *        init_weight(neural_cell *cell);
// static neural_cell *        set_functions(neural_cell *cell,
//                                                  struct transfer_library_function transfer,
//                                                  float (*summation)(vector *transfer),
//                                                  struct activation_library_function activation,
//                                                  struct cost_library_function error);
static neural_cell *        set_weight(neural_cell *cell, matrix *weight, float bias);
static neural_cell *        set_signal(neural_cell *cell, matrix *data);


/* Library Structure */
const struct neuron_library Neuron = {
    .create = cell_create,
    .delete = cell_delete,

    .context = {
        .layer = get_cell_layer,
        .create = context_create,
        .delete = context_delete
    },
    
    .weight = {
        .init = init_weight,
        .set = set_weight
    },
    
    .set = {
    //     .signal = set_signal
    //     .functions = set_functions
    },
    
    .fire = fire,
    .activation = activation,
    
    .synapse = {
        .read = collect_synapse_signal,
        .impulse = impulse
    }
};


/* Neural Cell in network */
static
neural_cell * 
cell_create(neuron_kernel nucleus, size_t layer, size_t position) {
    neural_cell *cell = malloc(sizeof(neural_cell));
    neuron_context *context = malloc(sizeof(neuron_context));
    
    context->layer_index = layer;
    context->position = position;
    
    context->prime = (struct neuron_state) {0};
    context->body = (struct neuron_state) {
        .weight     = Matrix.seed(Matrix.create(1, nucleus.transfer.dimension), 0),
        .bias       = random_range(-1, 1),
        .signal     = Matrix.create(1, 1), 
        .transfer   = Vector.create(1), 
        .activation = Vector.create(1),
        .error      = Vector.create(1)
    };

    *cell = (neural_cell) {
            .nucleus = nucleus, 
            .context = context,
            .axon = calloc(1, sizeof(neural_cell *)),
            .synapse = calloc(1, sizeof(neural_cell *)),
            .impulse_ready = calloc(1, sizeof(enum bool)),
            .feedback_ready = calloc(1, sizeof(enum bool))
        };

    return cell;
}

static
void
cell_delete(neural_cell *cell) {
    Neuron.context.delete(cell->context);
    free(cell->axon);
    free(cell->synapse);
    free(cell->impulse_ready);
    free(cell->feedback_ready);
    free(cell);

    *cell = (neural_cell){0};
}

/* Neuron context */
static 
neuron_context *
context_create(neural_cell *cell, neural_cell **layer_cells) {
    size_t layer_index = 0;
    vector ***layer_transfer = malloc(sizeof(vector**));
    vector ***layer_activation = malloc(sizeof(vector**));
    vector ***layer_error = malloc(sizeof(vector**));

    check_memory(layer_cells);
    neurons_check(layer_cells, "Layer for %zdx%zd cell building failed.", cell->context->layer_index, cell->context->position);

    while(layer_cells[layer_index]) {
        neural_cell *layer_cell = layer_cells[layer_index];
        struct neuron_state *body = &(layer_cell->context->body);

        neuron_ccheck(layer_cell, "Cell %zd from layer is broken", layer_index);
        size_t layer_size = (layer_index + 2) * sizeof(vector**);

        
        layer_transfer = realloc(layer_transfer, layer_size);
        check_memory(layer_transfer);
        layer_transfer[layer_index] = &(body->transfer);
        check_memory(layer_transfer[layer_index]);
        vector_check_print(*layer_transfer[layer_index],
                           "[%zd] Transfer vector is broken", layer_index);
        
        layer_activation = realloc(layer_activation, layer_size);
        check_memory(layer_activation);
        layer_activation[layer_index] = &(body->activation);
        check_memory(layer_activation[layer_index]);
        vector_check_print(*layer_activation[layer_index],
                           "[%zd] Activation vector is broken", layer_index);
        
        layer_error = realloc(layer_error, layer_size);
        check_memory(layer_error);
        layer_error[layer_index] = &(body->error);
        check_memory(layer_error);
        vector_check_print(*layer_error[layer_index],
                           "[%zd] Error vector is broken", layer_index);
        
        layer_index++;
        layer_transfer[layer_index] = NULL;
        layer_activation[layer_index] = NULL;
        layer_error[layer_index] = NULL;
    }
    
    cell->context->layer = (struct layer_state) {
        .dimension = layer_index,
        .transfer = layer_transfer,
        .activation = layer_activation,
        .error = layer_error

    };

    return cell->context;

error:
    free(layer_activation);
    free(layer_error);

    return cell->context; 
}

static
void
context_delete(neuron_context *context) {
    struct neuron_state *body = &context->body;
    
    Matrix.delete(body->signal);
    Matrix.delete(body->weight);
    
    Vector.delete(body->transfer);
    Vector.delete(body->activation);
    Vector.delete(body->error);   

    free(context->layer.transfer);
    free(context->layer.activation);
    free(context->layer.error);
    free(context);
}


/* Neuron Firing */
static
neural_cell *
fire(neural_cell *cell, matrix *data) {
    neuron_ccheck(cell, "Argument Cell");
    matrix_check_print(data, "For neuron fire");

    set_signal(cell, data);
    init_weight(cell);
    
    transfer(cell);
    fire_forward(cell);
    
    cell->activated = false;

    memset(cell->impulse_ready, 0, cell->context->body.signal->columns * sizeof(enum bool));
    return cell;

error:
    return NULL;
}

// Fire forward from source neuron to axon terminals
static
void
fire_forward(neural_cell *cell) {
    neuron_ccheck(cell, "Broken cell for fire forward");
    
    size_t index = 0;
    
    while (cell->axon[index]) {
        impulse(cell, cell->axon[index++]);
    }
    
error:
    return;
}

// Send impulse from source neuron to destination neuron
static
void
impulse(neural_cell *source, neural_cell *destination) {
    neuron_ccheck(source, "Broken source cell");
    neuron_ccheck(destination, "Broken destination cell");
    
    size_t index = 0;
    enum bool fire = true;
    enum bool impulse = false;
    
    while(destination->synapse[index]) {
        if(destination->synapse[index] == source) {
            destination->impulse_ready[index] = true;
            impulse = true;
        }
        
        if(destination->impulse_ready[index] == false) {
            fire = false;
        }
        
        if(impulse && fire == false){
            return;
        }
        
        index++;
    }
    
    if(fire) {
        matrix *signal = collect_synapse_signal(destination);

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
collect_synapse_signal(neural_cell *cell) {
    size_t dimension = 0;
    vector **impulse;
    neuron_ccheck(cell, "Broken cell from argument");
    
    impulse = malloc(cell->context->body.signal->columns * sizeof(vector *));
    while(cell->synapse[dimension]) {
        neuron_ccheck(cell->synapse[dimension], "Cell in synapse terminal");

        if (dimension > cell->context->body.signal->columns) {
            impulse = realloc(impulse, (dimension + 1) * sizeof(vector *));
            check_memory(impulse);
        }
        
        if(cell->synapse[dimension]->activated == false) {
            activation(cell->synapse[dimension]);
        }

        impulse[dimension] = cell->synapse[dimension]->context->body.activation;
        vector_check(impulse[dimension]);
        
        dimension++;
    }

    size_t bunch_size = impulse[0]->size;
    matrix *signal = Matrix.transpose(Matrix.from(impulse, dimension, bunch_size));
    matrix_check_print(signal, "Signal from bunch of vectors");
    
    free(impulse);
    
    return signal;
    
error:
    if(dimension > 0) { free(impulse); }
    return NULL;
}


/* Kernel */
// static
// neuron *
// set_functions(neuron *cell,
//                      struct transfer_library_function transfer,
//                      float (*summation)(vector *transfer),
//                      struct activation_library_function activation,
//                      struct cost_library_function error) {
//     cell->nucleus.transfer = transfer;
//     cell->nucleus.summation = summation;
//     cell->nucleus.activation = activation;
//     cell->nucleus.error = error;
// 
//     return cell;
// }

/* Weight */
static
neural_cell *
init_weight(neural_cell *cell) {
    struct neuron_state *body = &cell->context->body;
    neuron_state_check(*body, "Init weight");
    size_t dimension_diff = body->signal->columns - body->weight->rows;
    check(dimension_diff >= 0, "Signal columns should be more or equal to weight rows");
    
    if (dimension_diff > 0)
    {
        Matrix.reshape(body->weight, body->signal->columns, body->weight->columns);

        if (dimension_diff > 0) {
            matrix *weight = Matrix.seed(Matrix.create(dimension_diff, body->weight->columns),
                                         0);
            matrix_foreach(weight) {
                MATRIX(body->weight, body->weight->rows - row - 1, body->weight->columns - column - 1) = MATRIX(weight, row, column);
            }
            
            Matrix.delete(weight);
        }
    }

    matrix_check(body->weight);

    return cell;

error:
    return NULL;
}

static
neural_cell *
set_weight(neural_cell *cell, matrix *weight, float bias) {
    struct neuron_state *body = &cell->context->body;
    check_memory(body);
    matrix_check(weight);
    Matrix.delete(body->weight);

    body->weight = weight;
    body->bias = bias;
    
    return cell;

error:
    return NULL;
}

static
neural_cell *
set_signal(neural_cell *cell, matrix *data) {
    struct neuron_state *body = &cell->context->body;
    vector_values_check(data->vector);
    
    Matrix.delete(body->signal);
    body->signal = Matrix.copy((matrix*)data);
    matrix_check_print(body->signal, "Signal is broken");

    return cell;

error:
    return NULL;
}

static
neural_cell *
transfer(neural_cell *cell) {
    struct neuron_state *body = &cell->context->body;
    neuron_kernel *kernel = &cell->nucleus;
 
    Vector.delete(body->transfer);
    body->transfer = kernel->transfer.function(body->signal,
                                               body->weight,
                                               body->bias);
    vector_values_check(body->transfer);
    vector_check_print(cell->context->body.transfer, "Transfer is broken");
    
    return cell;
    
error:
    return NULL;
}

static
neural_cell *
activation(neural_cell *cell) {
    struct neuron_state *body = &cell->context->body;
    neuron_kernel *kernel = &cell->nucleus;
 
    vector *activation = kernel->activation.of(cell->context);
    Vector.delete(body->activation);
    body->activation = activation;
    vector_check_print(cell->context->body.activation, "Activation is broken");
    vector_values_check(body->activation);
    
    cell->activated = true;
    
    return cell;
    
error:
    return NULL;
}


/* Unused */
static
neural_cell **
get_cell_layer(neural_cell *cell) {
    neural_cell **layer_cells = calloc(1, sizeof(neural_cell*));
    size_t number_of_cells_in_layer = 0;

    check_memory(layer_cells);
    neuron_ccheck(cell, "Argument cell");

    neural_cell **terminal = cell->synapse;
    enum bool is_forward = false;
    if (*terminal == NULL){
        terminal = cell->axon;
        is_forward = true;
    }
    neurons_check(terminal, "Terminal array of cell %zdx%zd is broken", cell->context->layer_index, cell->context->position);
    for(size_t terminal_index = 0; terminal[terminal_index]; terminal_index++) {
        neural_cell *input_cell = terminal[terminal_index];
        neuron_ccheck(input_cell, "(%zdx%zd) Input cell TI = %zd", cell->context->layer_index, cell->context->position, terminal_index);

        neural_cell **input_cell_terminal = is_forward
            ? input_cell->synapse 
            : input_cell->axon;

        neurons_check(input_cell_terminal, "Terminal array of input cell %zdx%zd is broken", input_cell->context->layer_index, input_cell->context->position);
        for (size_t input_terminal_index = 0; input_cell_terminal[input_terminal_index]; input_terminal_index++)
        {
            neural_cell *axon_cell = input_cell_terminal[input_terminal_index];
            neuron_ccheck(axon_cell, "(%zdx%zd) Terminal cell TI = %zd, ITI = %zd", input_cell->context->layer_index, input_cell->context->position, terminal_index, input_terminal_index);
            
            size_t layer_cell_index = 0;
            neurons_check(layer_cells, "Uniq array of cells is broken");
            while(layer_cells[layer_cell_index]) {
                neural_cell *layer_cell = layer_cells[layer_cell_index];
                neuron_ccheck(layer_cell, "(%zdx%zd) Layer cell. TI = %zd, ITI = %zd, LCI = %zd", axon_cell->context->layer_index, axon_cell->context->position, terminal_index, input_terminal_index, layer_cell_index);
                
                if(layer_cell == axon_cell) {
                    break;
                }
                layer_cell_index++;
            }
            
            if(layer_cell_index == number_of_cells_in_layer) {
                layer_cells = realloc(layer_cells, (number_of_cells_in_layer + 2) * sizeof(neural_cell*));
                check_memory(layer_cells);
                layer_cells[number_of_cells_in_layer] = axon_cell;
                neuron_ccheck(layer_cells[number_of_cells_in_layer], "Uniq layer cell %zd", number_of_cells_in_layer);

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

