//
//  aggregation.h
//  math
//
//  Created by Alexandr Kondratyev on 07/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef aggregation_h
#define aggregation_h

#include <stdio.h>
#include "../../math/vector.h"

struct aggregation_library {
    float    (*sum)(vector *v);
    float    (*average)(vector *v);
    float    (*mean)(vector *v);
    float    (*mediana)(vector *v);
};

extern const struct aggregation_library Aggregation;


#endif /* aggregation_h */

