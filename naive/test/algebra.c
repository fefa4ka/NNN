//
//  algebra.c
//  naive
//
//  Created by Alexandr Kondratyev on 14/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "algebra.h"

#define assert(x, message) x ? log_ok(message) : log_error(message)

void vector_test_transpose() {
    vector *v = vector_create_ui();
    vector *w = vector_create_ui();
    
    matrix *V = Matrix.from(v, v->size, 1);
    matrix *W = Matrix.from(v, v->size, 1);
    
    matrix *vT = Matrix.transpose(Matrix.copy(V));
    matrix *wT = Matrix.transpose(Matrix.copy(W));
    
    matrix *vT_W = Matrix.mul(Matrix.copy(vT), W);
    matrix *wT_V = Matrix.mul(Matrix.copy(wT), V);
    
    assert(Matrix.rel.is_equal(vT_W, wT_V), "vT * w = wT * v");
    
    Vector.delete(v);
    Vector.delete(w);
    Matrix.delete(V);
    Matrix.delete(vT);
    Matrix.delete(vT_W);
    Matrix.delete(W);
    Matrix.delete(wT);
    Matrix.delete(wT_V);
}
