#include "unit.h"
#include <neural/network.h>
#include <neural/router.h>

neural_network   network;
neural_network   network_iris;
data_batch       iris_nn;

char *data_load()
{
    char *target_labels[] = {"species", NULL};
    data_set iris = Data.csv("/Users/fefa4ka/Development/math/math/dataset/iris.csv", NULL, target_labels);
    matrix *binary_target = Data.convert.vector_to_binary(iris.target.values->vector);
    data_set iris_binary = Data.matrix(iris.features.values, binary_target);
    iris_nn = Data.split(&iris_binary, 10, 90, 0, 10);

    return NULL;
}

char *neuron_life() {
    neuron_kernel kernel = {
        Transfer.linear,
        Aggregation.sum,
        Activation.relu,
        Cost.mean_squared,
        Optimization.sgd
    };

    //neuron body = Neuron.create(kernel);
    //NEURON_CHECK(body, "Initialized broken neuron");
    //Neuron.delete(&body);

    return NULL;
error:
    return "Neural cell broken";
}

char *network_test(neural_network *network){
    //test_assert(network->resolution.layers == 3, "Layers number is not as expected");
    //test_assert(network->resolution.size == 7, "Network size is different than expected %zd != %d", network->resolution.size, 7);
    network_check(network);
    
    return NULL;
error:
    return "Neuron cell broken";
}

char *network_create_for_or()
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
        Activation.soft_max,
        Cost.mean_squared,
        Optimization.sgd
    };
    
    neuron_kernel output = {
        Transfer.linear,
        Aggregation.sum,
        Activation.sigmoid,
        Cost.mean_squared,
        Optimization.sgd
    };

   neural_layer layers[] = {
        {
            .kernel = hidden,
            .router = Router.any,
            .dimension = 4 
        },
        {
            .kernel = output,
            .router = Router.any,
            .dimension = 1
        },

        { .dimension = 0 }
    };

    network = Network.create(layers);
    for (int try = 0; try < 100; try++) {
        Network.delete(&network);
        network = Network.create(layers);
        char *network_test_result = network_test(&network);
        test_assert(network_test_result == NULL, "%s", network_test_result);
    }

   return NULL;
}

char *network_create_for_iris()
{
    neuron_kernel input = {
        Transfer.linear,
        Aggregation.sum,
        Activation.tanh,
        Cost.mean_squared,
        Optimization.sgd
    };
    
    neuron_kernel hidden = {
        Transfer.linear,
        Aggregation.sum,
        Activation.relu,
        Cost.mean_squared,
        Optimization.sgd
    };
    
    neuron_kernel output = {
        Transfer.linear,
        Aggregation.sum,
        Activation.sigmoid,
        Cost.mean_squared,
        Optimization.sgd
    };

   neural_layer layers[] = {
        {
            .kernel = input,
            .router = Router.any,
            .dimension = 8
        },
        {
            .kernel = hidden,
            .router = Router.any,
            .dimension = 6
        },
        {
            .kernel = output,
            .router = Router.any,
            .dimension = 4
        },

        { .dimension = 0 }
    };

    network_iris = Network.create(layers);

   return NULL;
}

char *neuron_layer() {
     for (int try = 0; try < 1000; try++) {
        size_t random_index = (size_t)random_range(0, network.resolution.size);
        neural_cell *cell = network.neurons[random_index];
        neuron_ccheck(cell, "Random cell %zd from network broken", random_index);
        neural_cell **layer_neurons = Neuron.context.layer(cell);
        neurons_check(layer_neurons, "Layer for %zdx%zd neuron is broken", cell->coordinates.layer, cell->coordinates.position);
        free(layer_neurons);
     }

     return NULL;
error:
    return "Failed to get neuron layer";
}

char *neuron_create_context() {
    for (int try = 0; try < 1000; try++) {
        size_t random_index = (size_t)random_range(0, network.resolution.size);
        neural_cell *cell = network.neurons[random_index];
        neural_cell **layer_cells = Network.get.layer(&network, cell->coordinates.layer);
        // neuron_context *context = Neuron.context.create(cell, layer_cells);
        // Neuron.context.delete(context);
    }

    return NULL;
}

char *network_fire() {
    matrix *axon;
    for (size_t i = 0; i < 10000; i++)
    {
        axon = Network.fire(&network_iris, iris_nn.test->features.values);
        matrix_check(axon);
        test_assert(axon->rows == iris_nn.test->features.values->rows, "Sample size doesn't same in input and output.");
        test_assert(axon->columns == 4, "Axon dimension is not as expected");
        char *network_test_result = network_test(&network);
        test_assert(network_test_result == NULL, "%s", network_test_result);

        matrix *error = Network.error(&network, iris_nn.test->features.values, iris_nn.test->target.values, false);
        matrix_check(error);
        test_assert(error->rows == axon->rows, "Samples size of output and error is different %zd != %zd", axon->rows, error->rows);

        Matrix.delete(axon);
        Matrix.delete(error);

    }

    return NULL;
error:
    return "Broken signal from network axon";
}

char *network_error() {
    matrix *axon = axon = Network.fire(&network, iris_nn.test->features.values);
    matrix *error = Network.error(&network, iris_nn.test->features.values, iris_nn.test->target.values, false);
    matrix_check(axon);
    matrix_check(error);
    test_assert(error->rows == axon->rows, "Samples size of output and error is different %zd != %zd", axon->rows, error->rows);

    Matrix.delete(axon);
    Matrix.delete(error);
    
    return NULL;
error:
    return "Broken data";
}

char *or_train() {
    float or_train_data[] = {
        0., 0.,
        0., 1.,
        1., 0.,
        1., 1.,
    };
    float or_target_data[] = { 1., 0., 0., 0. };

    matrix *or_train = Matrix.from(Vector.from.floats(8, or_train_data), 4, 2);
    matrix *or_target = Matrix.from(Vector.from.floats(4, or_target_data), 4, 1);

    data_set or_data = Data.matrix(or_train, or_target);
    data_batch or_training_data = Data.split(&or_data, 0, 100, 0, 0);

    Network.train(&network, &or_training_data, 1, 10000);

    matrix *axon = Network.fire(&network, or_training_data.train->features.values);
    log_info("Predicted:");
    Matrix.print(axon);

    return NULL;
}

char *iris_train() {
    Network.train(&network_iris, &iris_nn, 0.01, 10000);

    return NULL;
}

char *all_tests() {
    test_init();

    test_run(neuron_life);
    test_run(network_create_for_or);
    test_run(network_create_for_iris);
    test_run(data_load);
    test_run(neuron_layer);
    test_run(neuron_create_context);    
    test_run(or_train);
    // test_run(iris_train);
    //test_run(network_fire);
    //test_run(network_error);

    return NULL;
}

RUN_TESTS(all_tests);
