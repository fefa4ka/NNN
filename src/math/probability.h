//
//  probability.h
//  math
//
//  Created by Alexandr Kondratyev on 02/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef probability_h
#define probability_h

#include <stdio.h>
#include "matrix.h"

#include "../util/sort.h"
#include "../data/csv.h"

#define PROBABILITY_TYPE "t_Prob"


typedef struct
{
    char *type;
    
    char **fields;
    matrix *samples;
    vector **events;
    vector **occurs;
    vector **P;
    float *variance;
    matrix *covariance;
    matrix *correlation;
} P_space;

struct probability_library {
    void          (*delete)(P_space *space);
    
    struct {
        P_space   (*vector)(vector *events, char *field);
        P_space   (*matrix)(matrix *events, char **fields);
        P_space   (*csv)(csv *data, char **fields);
    } from;
    
    
    struct {
        float    (*of)(P_space *space, char *field, float value);
        float    (*and)(P_space *space, char **fields, float *values);
        float    (*or)(P_space *space, char **fields, float *values);
        float    (*expected)(P_space *space, char *expected_field, char *related_field, float value);
    } mass;
    
    float    (*density)(P_space *space, float a, float b);
    float    (*conditional)(P_space *space, char *A_field, float A_value, char *B_field, float B_value);
    float    (*bayes)(P_space *space, char *A_field, float A_value, char *B_field, float B_value);
    
    float    (*expected)(P_space *space, char *field);
    float    (*variance)(P_space *space, char *field);
    float    (*covariance)(P_space *space, char *field, char *related_field);
    float    (*correlation)(P_space *space, char *field, char *related_field);
   
};

extern const struct probability_library Probability;

#endif /* probability_h */

