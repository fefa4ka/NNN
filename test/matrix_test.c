#include "unit.h"
#include <math/matrix.h>
#include <stdio.h>

matrix *M;

char *matrix_create() {
    M = Matrix.create(3, 6);

    test_assert(strcmp(M->type, MATRIX_TYPE) == 0, "Type is wrong");
    test_assert(M->rows == 3 && M->columns == 6 && M->vector->size == 3 * 6, "Size is different");
    
    return NULL;
}

char *matrix_delete() {
    Matrix.delete(M);

    return NULL; 
}

char *vector_transpose_test() {
    vector *v = Vector.seed(Vector.create(random_range(100, 10000)), 0);
    vector *w = Vector.seed(Vector.create(v->size), 0);
    vector_check(v);
    vector_check(w);

    matrix *V = Matrix.from(v, v->size, 1);
    matrix *W = Matrix.from(w, w->size, 1);
    matrix_check(V);
    matrix_check(W);

    matrix *vT = Matrix.transpose(Matrix.copy(V));
    matrix *wT = Matrix.transpose(Matrix.copy(W));
    matrix_check(vT);
    matrix_check(wT);

    matrix *vT_W = Matrix.mul(Matrix.copy(vT), W);
    matrix *wT_V = Matrix.mul(Matrix.copy(wT), V);
    matrix_check(vT_W);
    matrix_check(wT_V);

    test_assert(Matrix.rel.is_equal(vT_W, wT_V), "vT * w != wT * v");
     
    Vector.delete(v);
    Vector.delete(w);
    Matrix.delete(V);
    Matrix.delete(vT);
    Matrix.delete(vT_W);
    Matrix.delete(W);
    Matrix.delete(wT);
    Matrix.delete(wT_V);

    return NULL;
error:
    return "Transpose failed";
}

char *all_tests() {
    test_init();

    test_run(matrix_create);
    test_run(vector_transpose_test);
    test_run(matrix_delete);

    return NULL;
}

RUN_TESTS(all_tests);
