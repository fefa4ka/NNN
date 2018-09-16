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
#include "../../math/matrix.h"

struct transfer_library_function{
    vector *          (*samples)(matrix *input, matrix *weight, float bias);
    float             (*signal)(vector *input, matrix *weight, float bias);
    
    vector *          (*derivative)(vector *predicted, vector *target);
    vector *          (*prime)(matrix *weight);
    vector *          (*gradient)(matrix *samples,
                                  vector *target,
                                  vector *predicted);
    
    size_t            dimension;
};

struct transfer_library {
    struct transfer_library_function         transparent;
    struct transfer_library_function         linear;
};

extern const struct transfer_library Transfer;

#endif /* transfer_h */

