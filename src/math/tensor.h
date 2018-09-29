//
//  tensor.h
//  naive
//
//  Created by Alexandr Kondratyev on 21/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef tensor_h
#define tensor_h

#include <stdio.h>
#include "matrix.h"

#define TENSOR_INDEX(tensor, ...)
#define TENSOR_SIZE(tensor, RANK) Vector.sum.between(RANK, tensor->rank->size)
#define TENSOR_RANK_0(tensor, R_0) *((tensor)->vector->values + R_0 * TESOR_SIZE(tensor, 0))
#define TENSOR_RANK_1(tensor, R_0, R_1) *((tensor)->vector->values + R_0 * TESOR_SIZE(tensor, 0) + R_1 * TESOR_SIZE(tensor, 1))
#define TENSOR_RANK_2(tensor, R_0, R_1, R_2) *((tensor)->vector->values + R_0 * TESOR_SIZE(tensor, 0) + R_1 * TESOR_SIZE(tensor, 1) + R_2 * TESOR_SIZE(tensor, 2))


#define TENSOR_INDEX_N(tensor, rank, N, ...)
#define TENSOR_TYPE "t_Ten"

typedef struct
{
    char   *type;
    
    vector *rank;
    vector *data;
} tensor;


#endif /* tensor_h */
