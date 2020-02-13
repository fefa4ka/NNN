//
//  network.c
//  math
//
//  Created by Alexandr Kondratyev on 10/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "network.h"
#define NETWORK(network, layer, position) (network)->state[(int)Network.neuron(network, layer, position)]

static neural_network       create(neural_layer layers[]);
static void                 delete(neural_network *network);

static matrix *             fire(neural_network *network, matrix *signal);
static matrix *             axon(neural_network *network);
static size_t               get_neuron_position(neural_network *network, size_t layer, size_t position);
static neural_cell **       get_layer_cells(neural_network *network, size_t layer);
static float                compute_error(neural_network *network, matrix *signal, matrix *target);
static float                accuracy(neural_network *network,matrix *target);
static void                 cell_back_propagation(neural_cell *cell, float learning_rate);
static enum bool            neuron_error_impulse(neural_cell *source, neural_cell *destination);
static void                 train(neural_network *network, data_batch *training_data, float learning_rate, int epoch);
static float                back_propagation(neural_network *network, matrix *signal, matrix *target, float learning_rate);
static neural_network *     seed_next_layer(neural_network *network, neural_layer *layer);
static neural_network *     route(neural_network *network, neural_layer layers[]);
static void                 __build_cell_context(neural_network *network);

/* Library Structure */
const struct network_library Network = {
    .create = create,
    .delete = delete,
    .fire = fire,
    .train = train,
    .get = {
        .neuron = get_neuron_position,
        .layer = get_layer_cells
    },
    .error = compute_error
};

#define NETWORK_LAST_LAYER(network) \
    for(size_t layer = network->resolution.layers - 1; layer == network->resolution.layers - 1; layer++) \
        for(size_t position = 0; \
            position < network->resolution.dimensions[layer]; \
            position++)

static
neural_network
create(neural_layer layers[]) {
    neural_network network = {
        .resolution = {
            .layers = 0,
            .dimensions = malloc(sizeof(int)),
            .size = 0
        },
        .neurons = malloc(sizeof(neural_cell*)),
        .history = malloc(sizeof(network_loss))
    };
        
    size_t layer_index = 0;
    
    while(layers[layer_index].dimension != 0) {
        seed_next_layer(&network, &layers[layer_index++]);
    }
        
    route(&network, layers);   
    __build_cell_context(&network);
    
    return network;
}

static 
void
delete(neural_network *network) {
    free(network->resolution.dimensions);
    for (size_t index = 0; index < network->resolution.size; index++) {
        Neuron.delete(network->neurons[index]);
    }
    free(network->neurons);
}

/* Init layer neural cell instances */
static
neural_network *
seed_next_layer(neural_network *network, neural_layer *layer) {
    network->neurons = realloc(network->neurons,
                               (network->resolution.size + layer->dimension) * sizeof(neural_cell*));
    check_memory(network->neurons);
    network->resolution.dimensions = realloc(network->resolution.dimensions,
                                             (network->resolution.layers + 1) * sizeof(size_t));
    check_memory(network->resolution.dimensions);
    
    network->resolution.dimensions[network->resolution.layers] = layer->dimension;
    
    for(size_t position = 0; position < layer->dimension; position++) {
        neural_cell *cell_ptr = Neuron.create(layer->kernel, network->resolution.layers, position);
        network->neurons[network->resolution.size++] = cell_ptr;

        neuron_ccheck(network->neurons[network->resolution.size - 1], "Created broken cell %zdx%zd", network->resolution.layers, position);
    }
    
    network->resolution.layers++;
    
    return network;

error:
    return NULL;
}

/* Connect each layer cells using defined layer router */
static
neural_network *
route(neural_network *network, neural_layer layers[]) {
    size_t layer = network->resolution.layers;
    while(layer--) {
        layers[layer].router(network, layer);
    }
    
    return network;
}


/* Context */
static
void
__build_cell_context(neural_network *network) {
    for(size_t index = 0; index < network->resolution.size; index++) {
        neural_cell *cell = network->neurons[index];
        neuron_ccheck(cell, "Neuron %zd", index);
        neural_cell **layer_cells = get_layer_cells(network, cell->context->layer_index);

        Neuron.context.create(cell, layer_cells);

        free(layer_cells);
    }

error:
    return;
}

static
neural_cell **
get_layer_cells(neural_network *network, size_t layer) {
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
fire(neural_network *network, matrix *signal) {
    size_t index = 0;
    
    matrix_check_print(signal, "For network fire");

    while(network->resolution.size > index && network->neurons[index]->context->layer_index == 0) {
        neural_cell *cell = network->neurons[index];
        
        Neuron.fire(cell, signal);

        index++;
    }
    
    return axon(network);

error:
    return NULL;
}

/* Error */
static
float
compute_error(neural_network *network, matrix *signal, matrix *target) {
    matrix_check_print(signal, "Broken signal for network error");
    matrix_check_print(target, "Broken target for network error");

    size_t layer_index = network->resolution.layers - 1;
    size_t layer_size = network->resolution.dimensions[layer_index];
    size_t samples_count = 0;
    
    matrix *predicted = Network.fire(network, signal);
    float *error_body = malloc(layer_size * sizeof(float));
    vector **error_prime = malloc(layer_size * sizeof(vector*));
    check_memory(error_body);
    check_memory(error_prime);

    //#pragma omp parallel for
    for(size_t position = 0; position < layer_size; position++)
    {
        neural_cell *cell = &NEURON(network, layer_index, position);
        struct neuron_state *body = &cell->context->body;

        // Let's begin backpropagating the error derivatives. 
        // Since we have the predicted output of this particular input example, 
        // we can compute how the error changes with that output.
        error_prime[position] = cell->nucleus.error.derivative(cell->context,
                                                               target);
        if(cell->context->prime.error) {    
            Vector.delete(cell->context->prime.error);
        }
        cell->context->prime.error = error_prime[position];
 
        error_body[position] = cell->nucleus.error.of(cell->context,
                                                      target);
        VECTOR(body->error, 0) = error_body[position];

        if(body->activation->size > samples_count) {
            samples_count = body->activation->size;
        }
    }
    
    vector *error_vector = Vector.from.floats(layer_size, error_body);
    float error = Vector.sum.all(error_vector) / error_vector->size;

    // Garbage control
    Vector.delete(error_vector);
    Matrix.delete(predicted);
    free(error_prime);
    free(error_body);
    
    return error;

error:
    return 0;
}

/* Train */
static
void
train(neural_network *network, data_batch *training_data, float learning_rate, int epoch) {
    free(network->history);
    network->history = malloc(epoch * sizeof(network_loss));

    for (int epoch_index = 0; epoch_index < epoch; epoch_index++)
    {
        printf("Training %ld batches\n", training_data->count); 
        vector *train_error = Vector.create(training_data->count);
        vector *train_accuracy = Vector.create(training_data->count);

        for(size_t batch = 0; batch < training_data->count; batch++) {
            matrix *signal = training_data->mini[batch]->features.values;
            matrix *target = training_data->mini[batch]->target.values;

            matrix_check(signal);
            matrix_check(target);
            
            float batch_error = back_propagation(network, signal, target, learning_rate);
            float batch_accuracy = accuracy(network, target);

            VECTOR(train_error, batch) = batch_error;
            VECTOR(train_accuracy, batch) = batch_accuracy;
            
            if(batch)
                printf("\033[A\033[2K");
        
            printf("Training on %ld/%ld samples [Training Loss: %.10f | Accuracy: %.10f]\n", (batch + 1) * signal->rows, signal->rows * training_data->count, batch_error, batch_accuracy);

        }

        /* Calculate network perfomance */
        float train_error_mean = Vector.sum.all(train_error) / train_error->size;
        float train_accuracy_mean = Vector.sum.all(train_accuracy) / train_accuracy->size;

        matrix *validation_signal = training_data->validation->features.values;
        matrix *validation_target = training_data->validation->target.values;
        float validation_error = compute_error(network, validation_signal, validation_target);
        float validation_accuracy = accuracy(network, validation_target);
        
        if(epoch_index) {

            printf("\033[A\033[2K");
            printf("\033[A\033[2K");
            printf("\033[A\033[2K");
            printf("\033[A\033[2K");
        } else {
            printf("\033[A\033[2K");
            printf("\033[A\033[2K");
        }

        printf("Epoch %d [Validation Loss: %.10f | Accuracy: %.10f] [Training Loss: %.10f | Accuracy: %.10f]\n", epoch_index, validation_error, validation_accuracy, train_error_mean, train_accuracy_mean);
        

        /* Save perfomance result to history */
        network_loss *history = &network->history[epoch];

        history->train.error = train_error_mean;
        history->train.accuracy = train_accuracy_mean;

        history->validation.error = validation_error;
        history->validation.accuracy = validation_accuracy;

        Vector.delete(train_error);
        Vector.delete(train_accuracy);
    }

error:
    return;
}

static
float
accuracy(neural_network *network, matrix *target) {
    matrix *predicted = axon(network); 
    vector *target_vector = Data.convert.binary_to_vector(target);
    vector *predicted_vector = Data.convert.binary_to_vector(predicted);

    size_t predicted_count = 0;
    
    //#pragma omp parallel for reduction (+:predicted_count)
    for(size_t index = 0; index < target_vector->size; index++) {
        if((int)VECTOR(target_vector, index) == (int)VECTOR(predicted_vector, index)) {
            predicted_count += 1;
        }
    }

    float accuracy = predicted_count / (target_vector->size * 2. - predicted_count);

    return accuracy;
}

// The backpropagation algorithm decides how much to update each weight of the network 
// after comparing the predicted output with the desired output for a particular example.
static
float
back_propagation(neural_network *network, matrix *signal, matrix *target, float learning_rate) {
    matrix_check_print(signal, "Back propagate broken signal");
    matrix_check_print(target, "Back propagate broken target");

    // For this, we need to compute how the error changes with respect to each weigh.
    float error = compute_error(network, signal, target);
    
    NETWORK_LAST_LAYER(network) {
        neural_cell *cell = &NEURON(network, layer, position);
        
        cell_back_propagation(cell, learning_rate);
    }

    return error;

error:
    return 0;
}

/* Network Result Signal */
static
matrix *
axon(neural_network *network) {
    size_t layer_index = network->resolution.layers - 1;
    size_t layer_size = network->resolution.dimensions[layer_index];

    vector **axon = malloc(layer_size * sizeof(vector*));

    //#pragma omp parallel for
    for(size_t position = 0; position < layer_size; position++) {
        neural_cell *cell = &NEURON(network, layer_index, position);
        
        Neuron.activation(cell);
        axon[position] = cell->context->body.activation;
    }

    matrix *result = Matrix.transpose(Matrix.from(axon, layer_size, axon[0]->size));

    free(axon);

    return result;

}


// Back Propagation
static
void
cell_back_propagation(neural_cell *cell, float learning_rate) {
    neuron_ccheck(cell, "Broken cell from argument");
    check(learning_rate, "Learning rate doesn't set");

    static float *params = NULL;
    // TODO: Params for Adam
    params = cell->nucleus.optimization((void*)cell, learning_rate, params);
    // Synapse reverse fire
    size_t synapse_index = 0;
    while(cell->synapse[synapse_index]) {
        neural_cell *synapse_cell = cell->synapse[synapse_index];
        
        if(neuron_error_impulse(cell, synapse_cell)) {
            cell_back_propagation(synapse_cell, learning_rate);
        }
        synapse_index++;
    }

error:
    return;
}


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
            destination->feedback_ready[index] = true;
            impulse = true;
        }
        
        if(destination->feedback_ready[index] == false) {
            fire = false;
        }
        
        if(impulse && fire == false){
            return false;
        }
        
        index++;
    }
    
    if(fire) {
        free(destination->feedback_ready);
        destination->feedback_ready = calloc(index, sizeof(enum bool));
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
get_neuron_position(neural_network *network, size_t layer, size_t position) {
    size_t absolute_position = 0;
    
    while(layer--) {
        absolute_position += network->resolution.dimensions[layer];
    }
    
    return absolute_position + position;
}

