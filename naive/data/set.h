//
//  set.h
//  naive
//
//  Created by Alexandr Kondratyev on 14/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef set_h
#define set_h

#include <stdio.h>
#include "../math/probability.h"

typedef struct {
    csv *           data;
    
    struct {
        char **     labels;
        matrix *    values;
    } features;
    
    struct {
        char **     label;
        matrix *    values;
    } target;
    
    struct {
        double      mean;
        double      std;
    } normalization;
    
    P_space         probability;
} data_set;

struct data_library {
    data_set      (*csv)(char *filename, char** fields, char **target);
    void          (*delete)(data_set *data);
    void          (*print)(data_set *data);
};

extern const struct data_library Data;


#endif /* set_h */
