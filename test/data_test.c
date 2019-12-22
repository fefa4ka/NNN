#include "unit.h"
#include <data/set.h>

data_set set;
matrix *binary_target;

char *data_set_test(data_set set) {
    test_assert(set.data.fields != NULL && set.features.labels != NULL && set.target.labels != NULL, "Data set fields is NULL");
    matrix_check(set.data.values);
    matrix_check(set.features.values);
    matrix_check(set.target.values);

    return NULL;
error:
    return "Data set corrupted";
}

char *iris_load() {
    char *target_labels[] = {"species", NULL};
    set = Data.csv("/Users/fefa4ka/Development/math/math/dataset/iris.csv", NULL, target_labels);
    char *iris_test = data_set_test(set);
    test_assert(iris_test == NULL, "Iris data set corrupted");    

    return NULL;
}

char *vector_to_binary() {
    binary_target = Data.convert.vector_to_binary(set.target.values->vector);
    matrix_check(binary_target);

    return NULL;
error:
    return "To binary converting failed";
}

char *data_from_matrix() {
    data_set iris_binary = Data.matrix(set.features.values, binary_target);
    char *iris_binary_test = data_set_test(iris_binary);
    test_assert(iris_binary_test == NULL, "Binary data set target corrupted");
    Data.delete(&iris_binary);

    return NULL;
}

char *data_delete() {
    Data.delete(&set);
    Matrix.delete(binary_target);

    if(data_set_test(set)) {
        return NULL;
    } else {
        return "Data set exist after delete";
    }
}

char *all_tests() {
    test_init();

    test_run(iris_load);
    test_run(vector_to_binary);
    test_run(data_from_matrix);
    test_run(data_delete);
    
    return NULL;
}

RUN_TESTS(all_tests);
