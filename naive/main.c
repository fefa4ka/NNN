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

int main(int argc, const char * argv[]) {
    srand((unsigned int)time(NULL));

    neuron_kernel perceptron = {
        Transfer.linear,
        Aggregation.sum,
        Activation.tanh,
        Cost.square,
        Optimization.sgd
    };

    neuron_kernel another = {
        Transfer.linear,
        Aggregation.sum,
        Activation.relu,
        Cost.square,
        Optimization.sgd
    };
    
    neuron_kernel output = {
        Transfer.linear,
        Aggregation.sum,
        Activation.sigmoid,
        Cost.square,
        Optimization.sgd
    };

    neural_layer layers[] = {

        {
            .kernel = another,
            .router = Router.any,
            .dimension = 4
        },
        {
            .kernel = perceptron,
            .router = Router.any,
            .dimension = 2
        },
        {
            .kernel = output,
            .router = Router.any,
            .dimension = 1
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
    float or_target_data[] = { 0., 0., 1., 0. };
    matrix *or_train = Matrix.from(Vector.from.floats(8, or_train_data), 4, 2);
    matrix *or_target = Matrix.from(Vector.from.floats(4, or_target_data), 4, 1);

    // Neural network train
    neural_network network = Network.create(layers);
    
    Network.train(&network, or_train, or_target, 0.1);
    
    // Nn Test
    matrix *axon = Network.fire(&network, or_train);
    Matrix.print(axon);

    return 0;
}
