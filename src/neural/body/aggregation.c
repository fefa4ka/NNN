//
//  aggregation.c
//  math
//
//  Created by Alexandr Kondratyev on 07/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "aggregation.h"

static float aggregation_sum(vector *v);

/* Library structure */
const struct aggregation_library Aggregation = {
    .sum = aggregation_sum
};

static
float
aggregation_sum(vector *v) {
    return Vector.sum.all(v);
}


