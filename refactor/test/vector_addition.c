#include <math.h>
#include <math/vector.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define log_print(type, message, ...)                                          \
    printf(type "\t" message "\n", ##__VA_ARGS__)
#define log_debug(type, message, ...)                                          \
    log_print(type, message " (%s:%d)\n", ##__VA_ARGS__, __FILE__, __LINE__)
#define log_verbose(message, ...) log_print("VERBOSE", message, ##__VA_ARGS__)
#define log_info(message, ...)    log_print("INFO", message, ##__VA_ARGS__)
#define log_ok(message, ...)      log_print("OK", message, ##__VA_ARGS__)
#define log_error(message, ...)                                                \
    log_print("\e[1m\e[31mERROR\e[39m\e[0m", message " (%s:%d)\n",             \
              ##__VA_ARGS__, __FILE__, __LINE__)

#define profiler_tick()                                                        \
    {                                                                          \
        clock_t begin, end, passed;                                            \
        begin = clock()

#define profiler_tock()                                                        \
    end    = clock();                                                          \
    passed = end - begin;                                                      \
    printf("passed: %ld\n", passed);                                           \
    }
#define test_assert(test, message, ...)                                        \
    if (!(test)) {                                                             \
        log_error(message, ##__VA_ARGS__);                                     \
        return 1;                                                              \
    } else {                                                                   \
        log_ok(message, ##__VA_ARGS__);                                        \
    }


// Function to measure the time taken for a function to execute
clock_t measure_time(void (*func)(vector *, number *), vector *v, void *w)
{
    clock_t start, end, passed;

    const int num_trials = 1000;
    clock_t   total_time = 0;
    for (int i = 0; i < num_trials; i++) {
        start = clock();
        func(v, w);
        end    = clock();
        passed = end - start;
        total_time += passed;
    }
    return total_time / num_trials;
}


int main()
{
    vector *v = vector_from_list(3, (NNN_TYPE[]){1, 2, 3});
    vector *w = vector_from_list(3, (NNN_TYPE[]){4, 5, 6});
    vector *x = vector_from_list(3, (NNN_TYPE[]){7, 8, 9});
    vector *y = vector_from_list(3, (NNN_TYPE[]){10, 11, 12});
    vector *z = vector_from_list(3, (NNN_TYPE[]){13, 14, 15});

    vector *vw = vector_addition(v, w);
    vector *vwx = vector_addition(vw, x);
    vector *vwxy = vector_addition(vwx, y);
    vector *vwxyz = vector_addition(vwxy, z);

    test_assert(VECTOR(vwxy, 0) == 35, "xwxy[0] == 40");
    test_assert(VECTOR(vwxy, 1) == 40, "xwxy[1] == 40");
    test_assert(VECTOR(vwxy, 2) == 45, "xwxy[2] == 45");
    test_assert(v == vwxyz, "Addition don't create a copy");

    vector_delete(v);
    vector_delete(w);
    vector_delete(x);
    vector_delete(y);
    vector_delete(z);

    /* Test index of */
    vector *v1 = vector_from_list(3, (NNN_TYPE[]){1, 2, 3});
    test_assert(vector_index_of(v1, 2) == 1, "vector_index_of(v1, 2) == 1");
    test_assert(vector_index_of(v1, 3) == 2, "vector_index_of(v1, 3) == 2");
    test_assert(vector_index_of(v1, 4) == -1, "vector_index_of(v1, 4) == -1");
    vector_delete(v1);

    /* Test map */
    vector *v2 = vector_from_list(3, (NNN_TYPE[]){1, 2, 3});
    vector *v2_mapped = vector_map(v2, (NNN_TYPE(*)(NNN_TYPE))sqrt);
//    test_assert(VECTOR(v2_mapped,0) == 1., "v2_mapped[0] == %f", VECTOR(v2_mapped, 0));

    /* Test vector dot product */
    vector *v3 = vector_from_list(3, (NNN_TYPE[]){1, 2, 3});
    vector *w2 = vector_from_list(3, (NNN_TYPE[]){4, 5, 6});
    test_assert(vector_dot_product(v3, w2) == 32, "vector_dot_product(v3, w2) == 32");
    vector_delete(v3);
    vector_delete(w2);

    /* Test vector length that dot product of vector itself */
    vector *v4 = vector_from_list(3, (NNN_TYPE[]){1, 2, 3});
    test_assert(vector_length(v4) == sqrt(14), "vector_length(v4) == %f", vector_length(v4));
    vector_delete(v3);
    return 0;
}

