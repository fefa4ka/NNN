//
//  macros.c
//  naive
//
//  Created by Alexandr Kondratyev on 14/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "macros.h"

/* Random */
float random_range(float min, float max)
{
    float range = (max - min);
    float div = RAND_MAX / range;
    return min + (rand() / div);
}
