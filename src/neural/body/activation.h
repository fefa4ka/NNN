//
//  activation.h
//  math
//
//  Created by Alexandr Kondratyev on 02/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef activation_h
#define activation_h

#include <stdio.h>
#include <math.h>
#include "body.h"

struct activation_library_function {
    vector *      (*of)(neuron_context *context);
    vector *      (*derivative)(neuron_context *context);
};

struct activation_library {
    struct activation_library_function                 sigmoid;
    struct activation_library_function                 tanh;
    struct activation_library_function                 soft_sign;
    struct activation_library_function                 heaviside_step;
    struct activation_library_function                 soft_plus;
    struct activation_library_function                 soft_max;
    struct activation_library_function                 relu;
    struct activation_library_function                 leaky_relu;
    struct activation_library_function                 elu;
    struct activation_library_function                 transparent;
};

extern const struct activation_library Activation;

#endif /* activation_h */

