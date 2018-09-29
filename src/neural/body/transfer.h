//
//  transfer.h
//  math
//
//  Created by Alexandr Kondratyev on 05/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef transfer_h
#define transfer_h

#include <stdio.h>
#include "body.h"
#include "../../math/matrix.h"

struct transfer_library_function{
    vector *          (*function)(matrix *input, matrix *weight, float bias);
    matrix *          (*derivative)(neuron_context *context, enum bool by_weight);
    
    size_t            dimension;
};

struct transfer_library {
    struct transfer_library_function         linear;
};

extern const struct transfer_library Transfer;

#endif /* transfer_h */

