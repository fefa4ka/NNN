//
//  cost.h
//  math
//
//  Created by Alexandr Kondratyev on 02/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef cost_h
#define cost_h

#include <stdio.h>
#include "body.h"
#include "../../math/matrix.h"

struct cost_library_function {
    float         (*of)(neuron_context *context, matrix *target);
    vector *      (*derivative)(neuron_context *context, matrix *target);
};


struct cost_library {
    struct cost_library_function mean_squared;
    struct cost_library_function cross_entropy;
};

extern const struct cost_library Cost;

#endif /* cost_h */

