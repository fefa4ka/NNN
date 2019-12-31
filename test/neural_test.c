#include "unit.h"
#include <neural/network.h>
#include <neural/router.h>

neural_network   network;
data_batch       iris_data;

char *data_load()
{
    char *target_labels[] = {"species", NULL};
    data_set iris = Data.csv("./test/data/iris.csv", NULL, target_labels);
    matrix *binary_target = Data.convert.vector_to_binary(iris.target.values->vector);
    data_set iris_binary = Data.matrix(iris.features.values, binary_target);
    iris_data = Data.split(&iris_binary, 75, 90, 0, 10);

    return NULL;
}

char *network_create_for_iris()
{
    neuron_kernel input = {
        Transfer.linear,
        Aggregation.sum,
        Activation.relu,
        Cost.mean_squared,
        Optimization.sgd
    };
    
    neuron_kernel hidden = {
        Transfer.linear,
        Aggregation.sum,
        Activation.tanh,
        Cost.mean_squared,
        Optimization.sgd
    };
    
    neuron_kernel output = {
        Transfer.linear,
        Aggregation.sum,
        Activation.soft_max,
        Cost.cross_entropy,
        Optimization.sgd
    };

   neural_layer layers[] = {
         {
            .kernel = input,
            .router = Router.any,
            .dimension = 8 
        },

        {
            .kernel = output,
            .router = Router.any,
            .dimension = 3
        },

        { .dimension = 0 }
    };

    network = Network.create(layers);

   return NULL;
}

char *neuron_layer() {
     for (int try = 0; try < 1000; try++) {
        size_t random_index = (size_t)random_range(0, network.resolution.size);
        neural_cell *cell = network.neurons[random_index];
        neuron_ccheck(cell, "Random cell %zd from network broken", random_index);
        neural_cell **layer_neurons = Neuron.context.layer(cell);
        neurons_check(layer_neurons, "Layer for %zdx%zd neuron is broken", cell->context->layer_index, cell->context->position);
        free(layer_neurons);
     }

     return NULL;
error:
    return "Failed to get neuron layer";
}

char *network_fire() {
    matrix *axon;
    for (size_t i = 0; i < 10000; i++)
    {
        axon = Network.fire(&network, iris_data.test->features.values);
        matrix_check(axon);
        test_assert(axon->rows == iris_data.test->features.values->rows, "Sample size doesn't same in input and output.");
        test_assert(axon->columns == 4, "Axon dimension is not as expected");

        float error = Network.error(&network, iris_data.test->features.values, iris_data.test->target.values);

    }

    return NULL;
error:
    return "Broken signal from network axon";
}

char *network_error() {
    matrix *axon = axon = Network.fire(&network, iris_data.test->features.values);
    float error = Network.error(&network, iris_data.test->features.values, iris_data.test->target.values);

    return NULL;
error:
    return "Broken data";
}

char *iris_train() {
    Network.train(&network, &iris_data, 0.01, 200);
    
    matrix *axon = Network.fire(&network, iris_data.train->features.values);
    log_info("Predicted:");
    Matrix.print(axon);
    Matrix.print(iris_data.train->target.values);
    
    return NULL;
}

char *all_tests() {
    test_init();
    test_run(network_create_for_iris);
    test_run(data_load);
    test_run(neuron_layer);
    test_run(iris_train);

    test_run(network_fire);
    test_run(network_error);

    return NULL;
}

RUN_TESTS(all_tests);
