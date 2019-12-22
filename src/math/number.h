//
//  number.h
//  math
//
//  Created by Alexandr Kondratyev on 01/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef number_h
#define number_h

#include <math.h>
#include "../util/macros.h"

#define NUMBER_TYPE "t_Num"

#define PI 3.14159265358979323846

struct number_library {
    unsigned long (*gcd)(unsigned long u, unsigned long v);
    unsigned long (*euler_phi)(unsigned long size);
};

extern const struct number_library Number;


#endif /* number_h */
