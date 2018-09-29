#include "unit.h"
#include <math/matrix.h>

matrix *M;

char *matrix_create() {
    M = Matrix.create(3, 6);

    test_asset(M->type == MATRIX_TYPE, "Type is wrong");
    test_asset(M->rows == 3 && M->columns == 6 && M->vector->size == 3 * 6, "Size is different");
    
    return NULL;
}

char *matrix_delete() {
    Matrix.delete(M);

    test_asset(M != NULL, "Matris pointer is not NULL");

    return NULL;
}

char *all_tests() {
    test_init();

    test_run(matrix_create);
    test_run(matrix_delete);

    return NULL;
}

RUN_TESTS(all_tests);