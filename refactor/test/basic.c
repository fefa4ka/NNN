#include <math/vector.h> // include header for Linked Ring library
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

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
    clock_t total_time = 0;
    for (int i = 0; i < num_trials; i++) {
        start = clock();
        func(v, w);
        end        = clock();
        passed = end - start;
        total_time += passed;
    }
    return total_time /num_trials;
}

float
sigmoid(float value) {
    return 1.f / (1.f + exp(value * -1.f));
}

int main()
{
    int     r;
    vector *vector_tmp;
    v4sf tez = {1,2,3,4};

    printf("%f", tez[1]);

    tez[2] =32.;

    number *one_two_three;
    float   one_23 = 1.23;
    one_two_three  = float_create(1.23);
    test_assert(one_two_three, "Number allocated");
    test_assert(one_two_three->floated == one_23, "1.23 initialzed")

        r
        = number_delete(one_two_three);
    test_assert(r == 0, "Number deleted");

    vector *vector_2d;
    float   vec_2d_data[] = {1, 2};
    vector_2d             = vector_create(2);
    test_assert(vector_2d, "2D Vector created");
    VECTOR(vector_2d, 0) = vec_2d_data[0];
    VECTOR(vector_2d, 1) = vec_2d_data[1];
    test_assert(*(float *)vector_2d->number.values == vec_2d_data[0],
                "2D vector data written %f", VECTOR(vector_2d, 0));
    test_assert(((float *)vector_2d->number.values)[1] == vec_2d_data[1],
                "2D vector data written %f", VECTOR(vector_2d, 1));
    r = number_delete(vector_2d);
    test_assert(r == 0, "2D Vector deleted");

    vector *vector_3d;
    vector *vector_3d_copy;
    float   vec_3d_data[] = {3, 4, 5};

    vector_3d = vector_from_list(3, vec_3d_data);
    test_assert(vector_3d, "3D Vector created");
    test_assert(VECTOR(vector_3d, 0) == vec_3d_data[0],
                "3D vector data written %f", VECTOR(vector_3d, 0));
    test_assert(VECTOR(vector_3d, 1) == vec_3d_data[1],
                "3D vector data written %f", VECTOR(vector_3d, 1));
    test_assert(VECTOR(vector_3d, 2) == vec_3d_data[2],
                "3D vector data written %f", VECTOR(vector_3d, 2));

    vector_3d_copy = vector_clone(vector_3d);
    test_assert(VECTOR(vector_3d_copy, 0) == vec_3d_data[0],
                "3D vector data written %f", VECTOR(vector_3d, 0));
    test_assert(VECTOR(vector_3d_copy, 1) == vec_3d_data[1],
                "3D vector data written %f", VECTOR(vector_3d, 1));
    test_assert(VECTOR(vector_3d_copy, 2) == vec_3d_data[2],
                "3D vector data written %f", VECTOR(vector_3d, 2));

    r = number_delete(vector_3d);
    test_assert(r == 0, "3D Vector deleted");

    vector_tmp = vector_reshape(vector_3d_copy, 32);
    test_assert(vector_tmp, "Vector reshaped");
    test_assert(vector_3d_copy == vector_tmp, "Vector reshaped");
    test_assert(vector_3d_copy->length == 32, "Vector reshaped to length = %ld",
                vector_3d_copy->length);

    vector_3d = vector_clone(vector_3d_copy);
    test_assert(vector_3d, "Vector copied");
    test_assert(vector_3d->length == 32, "Vector copied with length = %ld",
                vector_3d->length);

    number *pi = float_create(3.14f);
    vector_tmp = vector_addition(vector_3d, pi);
    test_assert(VECTOR(vector_3d, 0) == (float)(vec_3d_data[0] + pi->floated),
                "Vector added %f + %f == %f", VECTOR(vector_3d, 0), pi->floated,
                (float)(vec_3d_data[0] + pi->floated));
    test_assert(VECTOR(vector_3d, 1) == (vec_3d_data[1] + pi->floated),
                "Vector added %f", VECTOR(vector_3d, 1));
    test_assert(VECTOR(vector_3d, 2) == (vec_3d_data[2] + pi->floated),
                "Vector added %f", VECTOR(vector_3d, 2));
    test_assert(VECTOR(vector_3d, 3) == pi->floated, "Vector added %f",
                VECTOR(vector_3d, 3));
    test_assert(VECTOR(vector_3d, 31) == pi->floated, "Vector added %f",
                VECTOR(vector_3d, 31));

    vector *big;
    vector *big_yet;
    number *one;
    one     = float_create(2.0);
    big     = vector_create(122880);
    big_yet = vector_create(122880);


    vector_addition(big, one);
    vector_addition(big_yet, one);
    vector_print(big);
    float naive, omp, dot;
    naive = vector_dot_product(big,big_yet);
    omp = vector_omp_dot_product(big,big_yet);
    dot = vector_bloated_dot_product(big,big_yet);
    log_ok("!!!!");
     clock_t time_taken_optimized;
 clock_t time_taken_naive;



  /* */

    time_taken_optimized = measure_time(vector_bloated_dot_product, big, big_yet);
    time_taken_naive = measure_time(vector_dot_product, big, big_yet);


    log_ok("vector_bloated_dot_product(vec, vec) = %f: %ld tick", dot, time_taken_optimized);
    log_ok("vector_dot_product(vec, vec) = %f: %ld tick", naive, time_taken_naive);
time_taken_naive = measure_time(vector_omp_dot_product, big, big_yet);
    log_ok("vector_omp_dot_product(vec, vec) = %f: %ld tick", omp, time_taken_naive);
    time_taken_optimized = measure_time(vector_addition, big, big_yet);
    time_taken_naive = measure_time(vector_naive_addition, big, big_yet);
  /* */

    log_ok("vector_addition(vec, vec): %ld tick", time_taken_optimized);
    log_ok("vector_naive_addition(vec, vec): %ld tick", time_taken_naive);

    time_taken_optimized = measure_time(vector_addition, big, one);
    time_taken_naive     = measure_time(vector_naive_addition, big, one);

    log_ok("vector_addition(vec, num): %ld tick", time_taken_optimized);
    log_ok("vector_naive_addition(vec, num): %ld tick", time_taken_naive);

/* */

    time_taken_optimized = measure_time(vector_substraction, big, big_yet);
    time_taken_naive = measure_time(vector_naive_substraction, big, big_yet);

    log_ok("vector_substraction(vec, vec): %ld tick", time_taken_optimized);
    log_ok("vector_naive_substraction(vec, vec): %ld tick", time_taken_naive);

    time_taken_optimized = measure_time(vector_substraction, big, one);
    time_taken_naive     = measure_time(vector_naive_substraction, big, one);

    log_ok("vector_substraction(vec, num): %ld tick", time_taken_optimized);
    log_ok("vector_naive_substraction(vec, num): %ld tick", time_taken_naive);

 /* */

    time_taken_optimized = measure_time(vector_multiplication, big, big_yet);
    time_taken_naive = measure_time(vector_naive_multiplication, big, big_yet);

    log_ok("vector_multiplication(vec, vec): %ld tick", time_taken_optimized);
    log_ok("vector_naive_multiplication(vec, vec): %ld tick", time_taken_naive);

    time_taken_optimized = measure_time(vector_multiplication, big, one);
    time_taken_naive     = measure_time(vector_naive_multiplication, big, one);

    log_ok("vector_multiplication(vec, num): %ld tick", time_taken_optimized);
    log_ok("vector_naive_multiplication(vec, num): %ld tick", time_taken_naive);

 /* */

    time_taken_optimized = measure_time(vector_division, big, big_yet);
    time_taken_naive = measure_time(vector_naive_division, big, big_yet);

    log_ok("vector_division(vec, vec): %ld tick", time_taken_optimized);
    log_ok("vector_naive_division(vec, vec): %ld tick", time_taken_naive);

    time_taken_optimized = measure_time(vector_division, big, one);
    time_taken_naive     = measure_time(vector_naive_division, big, one);

    log_ok("vector_division(vec, num): %ld tick", time_taken_optimized);
    log_ok("vector_naive_division(vec, num): %ld tick", time_taken_naive);

    time_taken_optimized = measure_time(vector_map, big, sigmoid);
    time_taken_naive = measure_time(vector_naive_map, big, sigmoid);

    log_ok("vector_map(vec, vec): %ld tick", time_taken_optimized);
    time_taken_optimized = measure_time(vector_omp_map, big, sigmoid);
    log_ok("vector_omp_map(vec, vec): %ld tick", time_taken_optimized);
    log_ok("vector_naive_map(vec, vec): %ld tick", time_taken_naive);

    return 0;
}
