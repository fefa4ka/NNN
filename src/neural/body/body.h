//
//  body.h
//  naive
//
//  Created by Alexandr Kondratyev on 21/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef body_h
#define body_h

#include <stdio.h>
#include "../../math/matrix.h"

struct neuron_state {
    matrix *                     weight;
    float                        bias;
    
    matrix *                     signal;
    
    vector *                     transfer;
    vector *                     activation;
    
    vector *                     error;
};

struct layer_state {
    vector **                    axon;
    vector **                    error;
    size_t                       dimension;
};

typedef struct {
    struct neuron_state          body;
    struct neuron_state          prime;
    struct layer_state           layer;
    float *                      variables;
} neuron_context;

#endif /* body_h */
