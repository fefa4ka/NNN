#undef NDEBUG
#ifndef unit_h
#define unit_h

#include <stdio.h>
#include <stdlib.h>
#include <util/macros.h>
#include <time.h>
#include <sys/resource.h>
#include <errno.h>

#define test_time_init() \
    float start_time, finish_time

#define test_memory_init() \
    struct rusage memory_usage; \
    size_t start_memory, finish_memory

#define test_init()       \
    char *message = NULL; \
    test_time_init();     \
    test_memory_init();

#define test_assert(test, message, ...)    \
    if (!(test))                           \
    {                                      \
        log_error(message, ##__VA_ARGS__); \
        return message;                    \
    }

#define test_time_tick(message)                   \
    start_time = (float)clock() / CLOCKS_PER_SEC; \
    getrusage(RUSAGE_SELF, &memory_usage);        \
    start_memory = memory_usage.ru_maxrss;        \
    printf("- \033[1m%s\033[0m\n", " " #message);

#define test_time_tock(message) \
    finish_time = (float)clock() / CLOCKS_PER_SEC; \
    finish_memory = memory_usage.ru_maxrss;        \
    printf(". %s took %f sec, %ld kb used\n",                   \
           " " #message,                           \
           finish_time - start_time,               \
           finish_memory - start_memory);              

#define test_try(tries) for (int try = 0; try < tries; try ++)

#define test_run(test)    \
    test_time_tick(test); \
    message = test();     \
    test_time_tock(test); \
    tests_run++;          \
    if (message)          \
        return message;

#define RUN_TESTS(name)                                                                        \
    int main(int argc, char *argv[])                                                           \
    {                                                                                          \
        srand((unsigned int)time(NULL));                                                       \
        argc = 1;                                                                              \
        printf("\033[1m[PENDING]\033[0m %s\n", argv[0]);                                       \
        char *result = name();                                                                 \
        if (result != 0)                                                                       \
        {                                                                                      \
            printf("[\e[1m\e[31mFAILED\e[39m\e[0m] \033[1m%s\033[0m - %s\n", argv[0], result); \
        }                                                                                      \
        else                                                                                   \
        {                                                                                      \
            printf("[\e[1m\e[32mPASSED\e[39m\e[0m] \033[1m%s\033[0m\n", argv[0]);              \
        }                                                                                      \
        printf("[TOTAL] %d tests\n", tests_run);                                               \
        exit(result != 0);                                                                     \
    }

int tests_run;


#endif
