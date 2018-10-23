//
//  network.c
//  math
//
//  Created by Alexandr Kondratyev on 10/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "network.h"
#define NETWORK(network, layer, position) (network)->state[(int)Network.neuron(network, layer, position)]

static neural_network       network_create(neural_layer layers[]);
static void                 network_delete(neural_network *network);

static matrix *             network_fire(neural_network *network, matrix *signal);
static matrix *             network_axon(neural_network *network);
static size_t               network_get_neuron_position(neural_network *network, size_t layer, size_t position);
static neural_cell **       network_get_layer_cells(neural_network *network, size_t layer);
static matrix *             network_error(neural_network *network, matrix *signal, matrix *target, enum bool derivative);
static void                 network_cell_back_propagation(neural_cell *cell, float learning_rate);
static enum bool            neuron_error_impulse(neural_cell *source, neural_cell *destination);
static void                 network_train(neural_network *network, data_batch *training_data, float learning_rate, int epoch);
static void                 network_back_propagation(neural_network *network, matrix *signal, matrix *target, float learning_rate);
static neural_network *     __network_seed_layer(neural_network *network, neural_layer *layer);
static neural_network *     __network_route(neural_network *network, neural_layer layers[]);
static void                 __network_build_cell_context(neural_network *network);

/* Library Structure */
const struct network_library Network = {
    .create = network_create,
    .delete = network_delete,
    .fire = network_fire,
    .train = network_train,
    .get = {
        .neuron = network_get_neuron_position,
        .layer = network_get_layer_cells
    },
    .error = network_error
};

#define NETWORK_LAST_LAYER(network) \
    for(size_t layer = network->resolution.layers - 1; layer == network->resolution.layers - 1; layer++) \
        for(size_t position = 0; \
            position < network->resolution.dimensions[layer]; \
            position++)

static
neural_network
network_create(neural_layer layers[]) {
    neural_network network = {
        .resolution = {
            .layers = 0,
            .dimensions = malloc(sizeof(int)),
            .size = 0
        },
        .neurons = malloc(sizeof(neural_cell*))
    };
        
    size_t layer_index = 0;
    
    while(layers[layer_index].dimension != 0) {
        __network_seed_layer(&network, &layers[layer_index++]);
    }
        
    __network_route(&network, layers);
        
   __network_build_cell_context(&network);
    
    return network;
}

static 
void
network_delete(neural_network *network) {
    free(network->resolution.dimensions);
    for (size_t index = 0; index < network->resolution.size; index++) {
        Neuron.cell.delete(network->neurons[index]);
    }
    free(network->neurons);
}

/* Network Seeding */
static
neural_network *
__network_seed_layer(neural_network *network, neural_layer *layer) {
    network->neurons = realloc(network->neurons,
                               (network->resolution.size + layer->dimension) * sizeof(neural_cell*));
    check_memory(network->neurons);
    network->resolution.dimensions = realloc(network->resolution.dimensions,
                                             (network->resolution.layers + 1) * sizeof(size_t));
    check_memory(network->resolution.dimensions);
    
    network->resolution.dimensions[network->resolution.layers] = layer->dimension;
    
    for(size_t position = 0; position < layer->dimension; position++) {
        neural_cell *cell_ptr = malloc(sizeof(neural_cell));
        *cell_ptr = Neuron.cell.create(layer->kernel, network->resolution.layers, position);
        network->neurons[network->resolution.size++] = cell_ptr;

        neuron_ccheck(network->neurons[network->resolution.size - 1], "Created broken cell %zdx%zd", network->resolution.layers, position);
    }
    
    network->resolution.layers++;
    
    return network;

error:
    return NULL;
}

static
neural_network *
__network_route(neural_network *network, neural_layer layers[]) {
    size_t layer = 0;
    while(network->resolution.layers > layer) {
        layers[layer].router(network, layer);
        
        layer++;
    }
    
    return network;
}


/* Context */
static
void
__network_build_cell_context(neural_network *network) {
    for(size_t index = 0; index < network->resolution.size; index++) {
        neural_cell *cell = network->neurons[index];
        neuron_ccheck(cell, "Neuron %zd", index);
        neural_cell **layer_cells = network_get_layer_cells(network, cell->coordinates.layer);

        cell->context = Neuron.context.create(cell, layer_cells);

        free(layer_cells);
    }

error:
    return;
}

static
neural_cell **
network_get_layer_cells(neural_network *network, size_t layer) {
    size_t layer_dimension = network->resolution.dimensions[layer];
    neural_cell **terminals = malloc((layer_dimension + 1) * sizeof(neural_cell*));
    check_memory(terminals);
    
    terminals[layer_dimension] = NULL;
    while(layer_dimension) {
        layer_dimension--;
        terminals[layer_dimension] = &NEURON(network, layer, layer_dimension);
        neuron_ccheck(terminals[layer_dimension], "Layer neurons pointers I = %zd", layer_dimension);
    }
    
    return terminals;

error:
    return NULL;
}

/* Network Fire */
static
matrix *
network_fire(neural_network *network, matrix *signal) {
    network_check(network);
    
    size_t index = 0;
    
    matrix_check_print(signal, "For network fire");

    while(network->resolution.size > index && network->neurons[index]->coordinates.layer == 0) {
        neural_cell *cell = network->neurons[index];
        
        Neuron.fire(cell, signal);
        
        index++;
    }
    
    return network_axon(network);

error:
    return NULL;
}

/* Error */
static
matrix *
network_error(neural_network *network, matrix *signal, matrix *target, enum bool derivative) {
    network_check(network);
    matrix_check_print(signal, "Broken signal for network error");
    matrix_check_print(target, "Broken target for network error");

    size_t layer_index = network->resolution.layers - 1;
    size_t layer_size = network->resolution.dimensions[layer_index];
    size_t position = 0;
    size_t error_dimension = 0;
    
    matrix *axon = network_fire(network, signal);
    vector **error = malloc(layer_size * sizeof(vector*));
    check_memory(error);
    while (layer_size > position)
    {
        neural_cell *cell = &NEURON(network, layer_index, position);
        vector *neuron_target = Matrix.column(target, position);
        
        if(derivative) {
            error[position] = cell->body.nucleus.error.derivative(cell->body.axon,
                                                                  neuron_target);
        } else {
            error[position] = cell->body.nucleus.error.of(cell->body.axon,
                                                          neuron_target);
        }
        vector_check(error[position]);
        
        if(cell->body.axon->size > error_dimension) {
            error_dimension = cell->body.axon->size;
        }
        
        Vector.delete(neuron_target);
        
        position++;
    }
    
    matrix *error_matrix = Matrix.transpose(Matrix.from(error, layer_size, error_dimension));
    matrix_check(error_matrix);
    
    // Garbage control
    position = 0;
    while(layer_size > position) {
        Vector.delete(error[position++]);
    }
    Matrix.delete(axon);
    free(error);
    
    return error_matrix;

error:
    return NULL;
}

/* Train */
static
void
network_train(neural_network *network, data_batch *training_data, float learning_rate, int epoch) {
    network_check(network);

    for (int epoch_index = 0; epoch_index < epoch; epoch_index++)
    {
        log_info("Epoch %d", epoch_index);
        for(size_t batch = 0; batch < training_data->count; batch++) {
            matrix *signal = training_data->mini[batch]->features.values;
            matrix *target = training_data->mini[batch]->target.values;
            matrix_check(signal);
            matrix_check(target);
            
            network_back_propagation(network, signal, target, learning_rate);
        }

        matrix *test_signal = training_data->test
            ? training_data->test->features.values
            : training_data->train->features.values;
        matrix *test_target = training_data->test
            ? training_data->test->target.values
            : training_data->train->target.values;
        matrix *error = network_error(network, test_signal, test_target, false);
        log_info("[Error: %.10f]", Vector.sum.all(error->vector) / error->rows);
        
        Matrix.delete(error);

        network_check(network);
    }

error:
    return;
}

static
void
network_back_propagation(neural_network *network, matrix *signal, matrix *target, float learning_rate) {
    matrix_check_print(signal, "Back propagate broken signal");
    matrix_check_print(target, "Back propagate broken target");

    matrix *error = network_error(network, signal, target, true);

    NETWORK_LAST_LAYER(network) {
        neural_cell *cell = &NEURON(network, layer, position);
        neuron_ccheck(cell, "Last layer neuron %zdx%zd is broken", layer, position);
        Vector.delete(cell->body.error);
        cell->body.error = Matrix.column(error, position);
        cell->context->body.error = cell->body.error;

        network_cell_back_propagation(cell, learning_rate);
    }
    Matrix.delete(error);

error:
    return;
}

/* Network Result Signal */
static
matrix *
network_axon(neural_network *network) {
    size_t layer_index = network->resolution.layers - 1;
    size_t layer_size = network->resolution.dimensions[layer_index];
    size_t position = 0;

    vector **axon = malloc(layer_size * sizeof(vector*));
    while(layer_size > position) {
        neural_cell *cell = &NEURON(network, layer_index, position);
        neuron_ccheck(cell, "Last layer broken cell");
        axon[position] = cell->body.axon;
        vector_check_print(axon[position], "Broken axon signal from last layer cell");
        position++;
    }

    matrix *result = Matrix.transpose(Matrix.from(axon, layer_size, axon[0]->size));

    free(axon);

    return result;
error:
    return NULL;
}


// Back Propagation
static
void
network_cell_back_propagation(neural_cell *cell, float learning_rate) {
    neuron_ccheck(cell, "Broken cell from argument");
    check(learning_rate, "Learning rate doesn't set");

    static float *params = NULL;
    // TODO: Params for Adam
    params = cell->body.nucleus.optimization((void*)cell, learning_rate, params);

    // Synapse reverse fire
    size_t synapse_index = 0;
    while(cell->synapse[synapse_index]) {
        neural_cell *synapse_cell = cell->synapse[synapse_index];
        neuron_ccheck(synapse_cell, "Broken cell in synapse terminal");
        
        if(neuron_error_impulse(cell, synapse_cell)) {
            network_cell_back_propagation(synapse_cell, learning_rate);
        }
        synapse_index++;
    }

error:
    return;
}

//static
//void
//network_cell_back_propagation(neural_cell *cell, float learning_rate) {
//    size_t axon_dimension = 0;
//    vector **errors = malloc(sizeof(vector*));
//    vector *cell_activation_prime = Vector.map(Vector.copy(cell->body.transfer),
//                                               cell->body.nucleus.activation.derivative);
//
//    while(cell->axon[axon_dimension]) {
//        neural_cell *axon_cell = cell->axon[axon_dimension];
//        matrix *axon_weight = Matrix.seed(Matrix.create(1, cell->body.nucleus.transfer.dimension),
//                                          1);
//        size_t weight_index = 0;
//        while(axon_cell->synapse[weight_index]) {
//            if(axon_cell->synapse[weight_index] == cell) {
//                for(size_t weight_dimension = 0; weight_dimension < axon_weight->columns; weight_dimension++) {
//                    MATRIX(axon_weight, 0, weight_dimension) = MATRIX(axon_cell->body.weight, weight_index, weight_dimension);
//                }
//                break;
//            }
//            weight_index++;
//        }
//
//        vector* transfer_prime = axon_cell->body.nucleus.transfer.prime(axon_weight);
//        errors = realloc(errors, (axon_dimension + 1) * sizeof(vector*));
//        errors[axon_dimension] = Vector.mul(Vector.copy(cell_activation_prime),
//                                            axon_cell->body.error);
//        Vector.num.mul(errors[axon_dimension],VECTOR(transfer_prime, 0));
//
//        axon_dimension++;
//
//        // Garbage Conrol
//        Matrix.delete(axon_weight);
//        Vector.delete(transfer_prime);
//    }
//
//    if(axon_dimension == 0) {
//        *errors = Vector.copy(cell->body.error);
//        axon_dimension = 1;
//    }
//
//    // Mean error from all axons
//    matrix *errors_matrix = Matrix.from(errors, axon_dimension, cell->body.signal->rows);
//    vector *error = Vector.create(cell->body.signal->rows);
//    for(size_t signal = 0; signal < cell->body.signal->rows; signal++) {
//        vector *dimension_signals = Matrix.column(errors_matrix, signal);
//       VECTOR(error, signal) = Vector.sum.all(dimension_signals) / dimension_signals->size;
//
//        Vector.delete(dimension_signals);
//    }
//    Vector.delete(cell->body.error);
//    cell->body.error = error;
//
//    // Update weights
////    float rate = learning_rate / error->size;
//    matrix *updated_weight = Matrix.mul(Matrix.transpose(Matrix.copy(cell->body.signal)),
//                                        error);
//    Vector.num.mul(updated_weight->vector, learning_rate);
//    Vector.num.div(updated_weight->vector, error->size);
//
//    cell->body.weight = Matrix.sub(cell->body.weight,
//                                   updated_weight);
//    cell->body.bias -= learning_rate * Vector.sum.all(error) / error->size;
//
//    // Garbage Conrol
//    for(size_t dimension = 0; dimension < axon_dimension; dimension++) {
//        Vector.delete(errors[dimension]);
//    }
//    Matrix.delete(errors_matrix);
//    Matrix.delete(updated_weight);
//    Vector.delete(cell_activation_prime);
//    free(errors);
//
//    // Synapse reverse fire
//    size_t synapse_index = 0;
//    while(cell->synapse[synapse_index]) {
//        neural_cell *synapse_cell = cell->synapse[synapse_index];
//        if(neuron_error_impulse(cell, synapse_cell)) {
//            network_cell_back_propagation(synapse_cell, learning_rate);
//        }
//        synapse_index++;
//    }
//}


static
enum bool
neuron_error_impulse(neural_cell *source, neural_cell *destination) {
    neuron_ccheck(source, "Broken source cell");
    neuron_ccheck(destination, "Broken destination cell");

    size_t index = 0;
    enum bool fire = true;
    enum bool impulse = false;
    
    while(destination->axon[index]) {
        if(destination->axon[index] == source) {
            destination->reverse[index] = true;
            impulse = true;
        }
        
        if(destination->reverse[index] == false) {
            fire = false;
        }
        
        if(impulse && fire == false){
            return false;
        }
        
        index++;
    }
    
    if(fire) {
        free(destination->reverse);
        destination->reverse = calloc(index, sizeof(enum bool));
        //        destination->error = memset(destination->error, 0, index * sizeof(int));
        return true;
    }
    
    return false;

error:
    return false;
}


/* Position */
static
size_t
network_get_neuron_position(neural_network *network, size_t layer, size_t position) {
    size_t absolute_position = 0;
    
    while(layer--) {
        absolute_position += network->resolution.dimensions[layer];
    }
    
    return absolute_position + position;
}

