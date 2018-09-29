
//
//  main.c
//  naive
//
//  Created by Alexandr Kondratyev on 14/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include <stdio.h>
#include "neural/network.h"
#include "neural/router.h"
#include <pthread.h>

int main(int argc, const char * argv[]) {
    srand((unsigned int)time(NULL));
    
    char *target_labels[] = {"species", NULL};
    data_set iris = Data.csv("/Users/fefa4ka/Development/math/math/dataset/iris.csv", NULL, target_labels);
    matrix *binary_target = Data.convert.vector_to_binary(iris.target.values->vector);
//    Matrix.print(binary_target);
    data_set iris_binary = Data.matrix(iris.features.values, binary_target);
//
//    char *target_label_binary[] = {"setosa" , "versicolor", "virginica", NULL};
//    for(int i = 0; i < 10000; i++) {
////        char *target_labels[] = {"species", NULL};
////        data_set iris = Data.csv("/Users/fefa4ka/Development/math/math/dataset/iris.csv", NULL, target_labels);
////        Data.delete(&iris);
////        csv *data = csv_readfile("/Users/fefa4ka/Development/math/math/dataset/iris.csv");
////        csv_delete(data);
//        data_set iris_binary = Data.matrix(iris.features.values, binary_target);
//        Data.delete(&iris_binary);
//    }
//
//
    
//    Data.print(&iris);
    data_batch iris_nn = Data.split(&iris_binary, 10, 90, 0, 10);
//    Matrix.print(iris_nn.mini[4]->target.values);
//    return 0;
//
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

    

    // Data
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
    
    // Neural network train
    neural_network network = Network.create(layers);
    
//    Network.train(&network, &iris_nn, 0.001, 10000);
//
//    // Nn Test
    matrix *axon = Matrix.create(1, 1);
    for(size_t i = 0; i < 1000; i++) {
        Matrix.delete(axon);
        axon = Network.fire(&network, iris_nn.test->features.values);
        
    }
    Matrix.print(axon);
    return 0;
}
