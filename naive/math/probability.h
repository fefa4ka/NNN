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
    double *variance;
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
        double    (*of)(P_space *space, char *field, double value);
        double    (*and)(P_space *space, char **fields, double *values);
        double    (*or)(P_space *space, char **fields, double *values);
        double    (*expected)(P_space *space, char *expected_field, char *related_field, double value);
    } mass;
    
    double    (*density)(P_space *space, double a, double b);
    double    (*conditional)(P_space *space, char *A_field, double A_value, char *B_field, double B_value);
    double    (*bayes)(P_space *space, char *A_field, double A_value, char *B_field, double B_value);
    
    double    (*expected)(P_space *space, char *field);
    double    (*variance)(P_space *space, char *field);
    double    (*covariance)(P_space *space, char *field, char *related_field);
    
};

extern const struct probability_library Probability;

#endif /* probability_h */

