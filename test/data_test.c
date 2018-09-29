#include "unit.h"
#include <data/set.h>

data_set set;
matrix *binary_target;

char *data_set_test(data_set set)
{
    test_asset(set.data.fields != NULL && set.features.labels != NULL && set.target.labels != NULL, "Data set fields is NULL");
    MATRIX_CHECK(set.data.values);
    MATRIX_CHECK(set.features.values);
    MATRIX_CHECK(set.target.values);

    return NULL;
error:
    return "Data set corrupted";
}

char *iris_load() {
    char *target_labels[] = {"species", NULL};
    set = Data.csv("/Users/fefa4ka/Development/math/math/dataset/iris.csv", NULL, target_labels);
    char *iris_test = data_set_test(set);
    test_asset(iris_test == NULL, "Iris data set corrupted");    

    return NULL;

error:
    return "Data loading failed";
}

char *vector_to_binary() {
    binary_target = Data.convert.vector_to_binary(set.target.values->vector);
    MATRIX_CHECK(binary_target);

    return NULL;
error:
    return "To binary converting failed";
}

char *data_from_marix() {
    data_set iris_binary = Data.matrix(set.features.values, binary_target);
    char *iris_binary_test = data_set_test(iris_binary);
    test_asset(iris_binary_test == NULL, "Binary data set target corrupted");

    return NULL;
}

char *all_tests() {
    test_init();

    test_run(iris_load);
    test_run(vector_to_binary);
    test_run(data_from_marix);

    return NULL;
}

RUN_TESTS(all_tests);