//
//  optimization.h
//  naive
//
//  Created by Alexandr Kondratyev on 16/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef optimization_h
#define optimization_h

#include <stdio.h>
#include "../cell.h"

struct optimization_library {
    optimization_function  sgd;
};

extern const struct optimization_library Optimization;

#endif /* optimization_h */
