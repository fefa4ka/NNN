#undef NDEBUG
#ifndef unit_h
#define unit_h

#include <stdio.h>
#include <stdlib.h>
#include <util/macros.h>
#include <time.h>

#define test_init()       \
    char *message = NULL; \
    float start_time, finish_time

#define test_assert(test, message, ...)    \
    if (!(test))                           \
    {                                      \
        log_error(message, ##__VA_ARGS__); \
        return message;                    \
    }

#define test_run(test)                             \
    start_time = (float)clock() / CLOCKS_PER_SEC;  \
    printf("- \033[1m%s\033[0m\n", " " #test);                \
    message = test();                              \
    finish_time = (float)clock() / CLOCKS_PER_SEC; \
    printf(". %s took %f sec\n",                \
           " " #test,                              \
           finish_time - start_time);              \
    tests_run++;                                   \
    if (message)                                   \
        return message;

#define RUN_TESTS(name)                                                                        \
    int main(int argc, char *argv[])                                                           \
    {                                                                                          \
        srand((unsigned int)time(NULL));                                                       \
        argc = 1;                                                                              \
        printf("\033[1m[RUNNING]\033[0m %s\n", argv[0]);                                       \
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