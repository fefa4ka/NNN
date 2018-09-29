#undef NDEBUG
#ifndef unit_h
#define unit_h

#include <stdio.h>
#include <stdlib.h>
#include <util/macros.h>

#define test_init() char *message = NULL

#define test_asset(test, message) \
    if (!(test))                  \
    {                             \
        log_error(message);       \
        return message;           \
    }

#define test_run(test)              \
    printf("\t - %s\n", " " #test); \
    message = test();               \
    tests_run++;                    \
    if (message)                    \
        return message;

#define RUN_TESTS(name)                          \
    int main(int argc, char *argv[])             \
    {                                            \
        argc = 1;                                \
        printf("[RUNNING] %s\n", argv[0]);       \
        char *result = name();                   \
        if (result != 0)                         \
        {                                        \
            printf("[FAILED] %s\n", result);     \
        }                                        \
        {                                        \
            printf("[PASSED] %s\n", argv[0]);    \
        }                                        \
        printf("[TOTAL] %d tests\n", tests_run); \
        exit(result != 0);                       \
    }

int tests_run;


#endif