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

struct activation_library_function {
    float      (*of)(float value);
    float      (*derivative)(float value);
};

struct activation_library_function_with_param {
    float      (*of)(float value, float param);
    float      (*derivative)(float value, float param);
};

struct activation_library {
    struct activation_library_function                 sigmoid;
    struct activation_library_function                 tanh;
    struct activation_library_function                 soft_sign;
    struct activation_library_function                 heaviside_step;
    struct activation_library_function                 soft_plus;
    struct activation_library_function                 relu;
    struct activation_library_function_with_param      leaky_relu;
    struct activation_library_function_with_param      elu;
    struct activation_library_function                 transparent;
};

extern const struct activation_library Activation;

#endif /* activation_h */

