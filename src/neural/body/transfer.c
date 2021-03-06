//
//  transfer.c
//  math
//
//  Created by Alexandr Kondratyev on 05/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "transfer.h"

static vector *     transfer_linear_function(matrix *input, matrix *weight, float bias);
static matrix *     transfer_linear_derivative(neuron_context *context, enum bool by_weight);


/* Library structure */
const struct transfer_library Transfer = {
    .linear = {
        .function = transfer_linear_function,
        .derivative = transfer_linear_derivative,
        
        .dimension = 1
    }
};

/* Linear functions */
static
vector *
transfer_linear_function(matrix *input, matrix *weight, float bias) {
    matrix *transfer_matrix = Matrix.mul(Matrix.copy(input), weight->vector);
    Vector.num.add(transfer_matrix->vector, bias);
    
    vector *transfer = Vector.copy(transfer_matrix->vector);
    Matrix.delete(transfer_matrix);
    
    return transfer;
}

static
matrix *
transfer_linear_derivative(neuron_context *context, enum bool by_weight) {
    if(by_weight) {
        return Matrix.copy(context->body.signal);
    } else {
        return Matrix.copy(context->body.weight);
    }
}


