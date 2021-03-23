#include "unit.h"
#include <data/set.h>

csv *groceries;

char *groceries_load() {
    groceries = Csv.from.file("./test/data/Groceries_dataset.csv");
    test_assert(groceries != NULL, "Groceries data set corrupted");

    return NULL;
}

char *groceries_delete() {
    Csv.delete(groceries);

    return NULL;
}

char *all_tests() {
    test_init();

    test_run(groceries_load);
    test_run(groceries_delete);
    
    return NULL;
}

RUN_TESTS(all_tests);
