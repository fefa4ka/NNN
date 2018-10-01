#include "unit.h"
#include <math/matrix.h>

matrix *M;

char *matrix_create() {
    M = Matrix.create(3, 6);

    test_assert(M->type == MATRIX_TYPE, "Type is wrong");
    test_assert(M->rows == 3 && M->columns == 6 && M->vector->size == 3 * 6, "Size is different");
    
    return NULL;
}

char *matrix_delete() {
    Matrix.delete(M);

    MATRIX_CHECK(M);

    return "Matrix valid after delete";
error:
    return NULL;
}

char *vector_transpose_test() {
    vector *v = Vector.create(random_range(1, 1000));
    vector *w = Vector.create(random_range(1, 1000));
    VECTOR_CHECK(v);
    VECTOR_CHECK(w);

    matrix *V = Matrix.from(v, v->size, 1);
    matrix *W = Matrix.from(v, v->size, 1);
    MATRIX_CHECK(V);
    MATRIX_CHECK(W);

    matrix *vT = Matrix.transpose(Matrix.copy(V));
    matrix *wT = Matrix.transpose(Matrix.copy(W));
    MATRIX_CHECK(vT);
    MATRIX_CHECK(wT);

    matrix *vT_W = Matrix.mul(Matrix.copy(vT), W);
    matrix *wT_V = Matrix.mul(Matrix.copy(wT), V);
    MATRIX_CHECK(vT_W);
    MATRIX_CHECK(wT_V);

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