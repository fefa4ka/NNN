//
//  cost.c
//  math
//
//  Created by Alexandr Kondratyev on 02/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "cost.h"

static vector *       cost_loss_square_vector(vector *predicted, vector *target);
static vector *       cost_loss_cross_entropy_vector(vector *predicted, vector *target);

static vector *       cost_loss_square_vector_derivative(vector *predicted, vector *target);
static vector *       cost_loss_cross_entropy_vector_derivative(vector *predicted, vector *target);

/* Library structure */
const struct cost_library Cost = {
    //    .loss = {
    .square = {
        .of = cost_loss_square_vector,
        .derivative = cost_loss_square_vector_derivative
    },
    .cross_entropy = {
        .of = cost_loss_cross_entropy_vector,
        .derivative = cost_loss_cross_entropy_vector_derivative
    },
    //    }
};

static
float cost_loss_square_scalar(float predicted, float target) {
    return pow((predicted - target), 2) / 2;
}

static
float cost_loss_square_scalar_derivative(float predicted, float target) {
    return predicted - target;
}

static
vector *
cost_loss_square_vector(vector *predicted, vector *target) {
    vector *loss = Vector.sub(Vector.copy(predicted), target);
    loss = Vector.mul(loss, loss);
    loss = Vector.num.div(loss,
                          2);
    
    return loss;
}

static
vector *
cost_loss_square_vector_derivative(vector *predicted, vector *target) {
    vector *loss = Vector.num.mul(Vector.sub(Vector.copy(predicted),
                                             target),
                                  2);
    
    return loss;
}


static
float cost_loss_cross_entropy(float predicted, float target) {
    return -1. * (target * log(predicted) + (1. - target) * log(1. - predicted));
}

static
float
cost_loss_cross_entropy_derivative(float predicted, float target) {
    return -1. * target / predicted + (1 - target) / (1 - predicted);
}

static
vector *
cost_loss_cross_entropy_vector_derivative(vector *predicted, vector *target) {
//    float rate = 0.00001;
//    float one = 1;
//    float minus_one = -1;
//    vector *loss = Vector.add(Vector.div(Vector.copy(target), predicted),
//                              Vector.div(Vector.add(Vector.mul(Vector.copy(target), &minus_one), &one),
//                                         Vector.add(Vector.mul(Vector.copy(predicted),
//                                                               &minus_one),
//                                                    &one)));
    return target;
}

static
vector *
cost_loss_cross_entropy_vector(vector *predicted, vector *target) {
//    vector *logPredicted = Vector.map(Vector.copy(predicted), log);
//
//    vector *oneMinusOriginal = Vector.mul.num(
//                                              Vector.add.num(Vector.copy(target), -1.),
//                                              -1.
//                                              );
//
//    vector *logOneMinusPredicted = Vector.map(
//                                              Vector.mul.num(
//                                                             Vector.add.num(Vector.copy(predicted), -1.),
//                                                             -1.
//                                                             ),
//                                              log
//                                              );
//
//    vector *loss = Vector.mul.num(
//                                  Vector.add.vec(
//                                                 Vector.mul.vec(Vector.copy(logPredicted), predicted),
//                                                 Vector.mul.vec(oneMinusOriginal, logOneMinusPredicted)
//                                                 ),
//                                  -1.
//                                  );
//
//    Vector.delete(logPredicted);
//    Vector.delete(oneMinusOriginal);
//    Vector.delete(logOneMinusPredicted);
    
    return target;
}


