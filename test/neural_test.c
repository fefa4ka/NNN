#include "unit.h"
#include <neural/cell.h>
#include <neural/router.h>

neural_network   network;
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

char *network_test(neural_network *network){
    test_assert(network->resolution.layers == 3, "Layers number is not as expected");
    test_assert(network->resolution.size == 34, "Network size is different than expected %zd != %d", network->resolution.size, 34);

    for (size_t index = 0; index < network->resolution.size; index++) {
        NEURON_CELL_CHECK(&network->neurons[index], "Neuron cell %zd is broken", index);
    }

    return NULL;
error:
    return "Neuron cell broken";
}

char *network_create()
{
    neuron_kernel perceptron = {
        Transfer.linear,
        Aggregation.sum,
        Activation.tanh,
        Cost.mean_squared,
        Optimization.sgd
    };

    neuron_kernel another = {
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
            .kernel = perceptron,
            .router = Router.any,
            .dimension = 30
        },
        {
            .kernel = perceptron,
            .router = Router.any,
            .dimension = 2
        },
        {
            .kernel = output,
            .router = Router.any,
            .dimension = 2
        },

        { .dimension = 0 }
    };

    network = Network.create(layers);
    char *network_test_result = network_test(&network);

    test_assert(network_test_result == NULL, "%s", network_test_result);

    return NULL;
}

char *network_fire() {
    matrix *axon;
    for (size_t i = 0; i < 10000; i++)
    {
        axon = Network.fire(&network, iris_nn.test->features.values);
        MATRIX_CHECK(axon);
        test_assert(axon->rows == iris_nn.test->features.values->rows, "Sample size doesn't same in input and output.");
        test_assert(axon->columns == 2, "Axon dimension is not as expected");
        char *network_test_result = network_test(&network);
        test_assert(network_test_result == NULL, "%s", network_test_result);

        matrix *error = Network.error(&network, iris_nn.test->features.values, iris_nn.test->target.values, false);
        MATRIX_CHECK(error);
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
    MATRIX_CHECK(axon);
    MATRIX_CHECK(error);
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
    float or_target_data[] = { 0., 1., 1., 0. };

    matrix *or_train = Matrix.from(Vector.from.floats(8, or_train_data), 4, 2);
    matrix *or_target = Matrix.from(Vector.from.floats(4, or_target_data), 4, 1);

    data_set or_data = Data.matrix(or_train, or_target);
    data_batch or_training_data = Data.split(&or_data, 0, 100, 0, 0);

    Network.train(&network, &or_training_data, 0.001, 10000);

    return NULL;
}

char *all_tests() {
    test_init();

    test_run(network_create);
    test_run(data_load);
    test_run(network_fire);
    test_run(network_error);
    test_run(or_train);

    return NULL;
}

RUN_TESTS(all_tests);