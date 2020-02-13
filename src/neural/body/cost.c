//
//  cost.c
//  math
//
//  Created by Alexandr Kondratyev on 02/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "cost.h"

static float          mean_squared(neuron_context *context, matrix *target);
static vector *       mean_squared_derivative(neuron_context *context, matrix *target);
// static vector *       cost_loss_mean_squared(vector *predicted, vector *target);
static vector *       cost_loss_mean_squared_derivative(vector *predicted, vector *target);

static float          cross_entropy(neuron_context *context, matrix *target);
static vector *       cross_entropy_derivative(neuron_context *context, matrix *target);
static float          cost_loss_cross_entropy_vector(vector *predicted, vector *target);
static vector *       cost_loss_cross_entropy_vector_derivative(vector *predicted, vector *target);

/* Library structure */
const struct cost_library Cost = {
    .mean_squared = {
        .of = mean_squared,
        .derivative = mean_squared_derivative
    },
    .cross_entropy = {
        .of = cross_entropy,
        .derivative = cross_entropy_derivative
    },
};


// Mean Squared Error
static
float
mean_squared(neuron_context *context, matrix *target) {
    vector *predicted = Vector.copy(context->body.activation);
    vector *cell_target = Matrix.column(target, context->position);

    vector *loss = Vector.sub(predicted, cell_target);
    loss = Vector.mul(loss, loss);
 
    float mse = Vector.sum.all(loss) / loss->size;

    Vector.delete(cell_target);
    Vector.delete(loss);
    Vector.delete(target);

    return mse;
}


static
vector *
mean_squared_derivative(neuron_context *context, matrix *target) {
    vector *predicted = context->body.activation;
    vector *cell_target = Matrix.column(target, context->position);

    vector *loss = cost_loss_mean_squared_derivative(predicted, cell_target);

    Vector.delete(target);

    return loss;
}


/* Unused 
static
vector *
cost_loss_mean_squared(vector *predicted, vector *target) {
    vector *loss = Vector.sub(Vector.copy(predicted), target);
    loss = Vector.mul(loss, loss);
    loss = Vector.num.div(loss,
                          2);
    
    return loss;
}
*/

static
vector *
cost_loss_mean_squared_derivative(vector *predicted, vector *target) {
    vector *loss = Vector.sub(Vector.copy(predicted),
                              target);
    
    return loss;
}



// Cross Entropy
/* Unused
static
float cost_loss_cross_entropy(float predicted, float target) {
  //   "%s * (%M * log(%v) * (%s - %M) * log(%s - %v)"
    return -1. * (target * log(predicted) + (1. - target) * log(1. - predicted));
}

static
float
cost_loss_cross_entropy_derivative(float predicted, float target) {
    return -1. * (target / predicted + (1 - target) / (1 - predicted));
}

static
vector *
cost_loss_cross_entropy_vector_derivative(vector *predicted, vector *target) {
    float rate = 0.00001;
    float one = 1;
    float minus_one = -1;
    vector *loss = Vector.add(Vector.div(Vector.copy(target), predicted),
                              Vector.div(Vector.add(Vector.mul(Vector.copy(target), &minus_one), &one),
                                         Vector.add(Vector.mul(Vector.copy(predicted),
                                                               &minus_one),
                                                    &one)));
    return target;
}
*/

static 
double log0(double number) {
    return log(number + 1e-15); 
}


static
float
cross_entropy(neuron_context *context, matrix *target) {
    size_t layer_size = context->layer.dimension;
    size_t samples_count = 0;

    vector **predicted = malloc(layer_size * sizeof(vector*));

    for(size_t index = 0; index < layer_size; index++) {
        vector *cell_predicted = *context->layer.activation[index];

        predicted[index] = cell_predicted; 

        if(cell_predicted->size > samples_count) {
            samples_count = cell_predicted->size;
        }
    }

    matrix *predicted_matrix = Matrix.from(predicted, layer_size, samples_count); 
    //matrix *predicted_matrix = Matrix.transpose(Matrix.from(predicted, layer_size, samples_count)); 
    matrix *target_matrix = Matrix.transpose(Matrix.copy(target));
    
    vector *neurons_loss = Vector.create(samples_count);

    //#pragma omp parallel for
    for(size_t index = 0; index < samples_count; index++) {
        vector *sample_predicted = Matrix.column(predicted_matrix, index);
        vector *sample_target = Matrix.column(target_matrix, index);
        
        VECTOR(neurons_loss, index) = cost_loss_cross_entropy_vector(sample_predicted, sample_target);
        
        Vector.delete(sample_predicted);
        Vector.delete(sample_target);
    }


    float loss = Vector.sum.all(neurons_loss) / neurons_loss->size * -1.;
    
    Matrix.delete(predicted_matrix);
    Matrix.delete(target_matrix);
    free(predicted);

    Vector.delete(neurons_loss);
    
    return loss;
}


static
vector *
cross_entropy_derivative(neuron_context *context, matrix *target) {
    vector *predicted = context->body.activation;
    vector *cell_target = Matrix.column(target, context->position);

    vector *loss = cost_loss_cross_entropy_vector_derivative(predicted, cell_target);
    
    Vector.delete(cell_target);
   
    return loss;

}

static
float
cost_loss_cross_entropy_vector(vector *predicted, vector *target) {
    vector *logPredicted = Vector.map(Vector.copy(predicted), log0);

    float loss = Vector.dot(target, logPredicted);

    Vector.delete(logPredicted);

    return loss;

}

/* Unused
static
vector *
cost_loss_cross_entropy_vector_binary(vector *predicted, vector *target) {
    vector *logPredicted = Vector.map(Vector.copy(predicted), log0);
    vector *oneMinusTarget = Vector.num.add(
                                              Vector.num.mul(Vector.copy(target), -1.),
                                              1.);
    vector *logOneMinusPredicted = Vector.map(
                                              Vector.num.add(
                                                             Vector.num.mul(Vector.copy(predicted), -1.),
                                                             1.),
                                              log0);
    vector *loss = Vector.num.mul(
                              Vector.add(
                                         Vector.mul(Vector.copy(logPredicted), target),
                                         Vector.mul(oneMinusTarget, logOneMinusPredicted)),
                              -1.);

    // Garbage Control
    Vector.delete(logPredicted);
    Vector.delete(oneMinusTarget);
    Vector.delete(logOneMinusPredicted);
    
    return loss;

}
*/

static
vector *
cost_loss_cross_entropy_vector_derivative(vector *predicted, vector *target) {
    vector *predictedSafe = Vector.num.sub(Vector.copy(predicted), 10e-5);
    vector *targetOverPredicted = Vector.num.mul(
                                                Vector.div(Vector.copy(target),
                                                           predictedSafe),
                                                -1.);
    vector *oneMinusTarget = Vector.num.add(
                                              Vector.num.mul(Vector.copy(target), -1.),
                                              1.);
    vector *oneMinusPredicted = Vector.num.add(
                                              Vector.num.mul(Vector.copy(predictedSafe), -1.),
                                              1.);
    vector *oneMinusDivision = Vector.div(Vector.copy(oneMinusTarget), 
                                          oneMinusPredicted);
    
    vector *loss = Vector.add(Vector.copy(targetOverPredicted), 
                              oneMinusDivision);

    // Garbage Control
    Vector.delete(predictedSafe);
    Vector.delete(targetOverPredicted);
    Vector.delete(oneMinusTarget);
    Vector.delete(oneMinusPredicted);
    Vector.delete(oneMinusDivision);


    return loss;

}
