//
//  macros.h
//  naive
//
//  Created by Alexandr Kondratyev on 14/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef macros_h
#define macros_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#define DEBUG 1

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define log_print(type, message, ...) do { if(DEBUG) printf("[" type "] " message "\n", ##__VA_ARGS__); } while(0)
#define debug_print(type, message, ...) log_print(type, message " (%s:%d)\n", ##__VA_ARGS__, __FILENAME__, __LINE__)

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_info(message, ...) log_print("INFO", message, ##__VA_ARGS__)
#define log_ok(message, ...) log_print("OK", message, ##__VA_ARGS__)
#define log_error(message, ...) debug_print("\e[1m\e[31mERROR\e[39m\e[0m", "[errno: %s] " message, clean_errno(), ##__VA_ARGS__)
#define log_warning(message, ...) log_print("\e[33mWARNING\e[39m", "[errno: %s] " message, clean_errno(), ##__VA_ARGS__)
#define push_error(message, ...) { log_error(message, ##__VA_ARGS__); errno = 0; goto error; }

#define check(expression, message, ...) do { if(DEBUG && !(expression)) push_error(message, ##__VA_ARGS__) } while(0)
#define check_memory_print(variable, message, ...) check((variable), "Out of memory. " message, ##__VA_ARGS__)
#define check_memory(variable) check_memory_print(variable, "")
#define check_debug(expression, message, ...) if(!(expression)) { debug(message, ##__VA_ARGS__); }
#define sentinel(message, ...) push_error(message, ##__VA_ARGS__)


// Weird predefined struct type cheker
#define TYPE_INDEX 116 // "t" = 116 t_Vector, t_Matrix
#define IS(type, typeName) (*(char*)type != 0 && (*(*(char**)type)) == TYPE_INDEX && strcmp(*(char **)type, typeName) == 0)


#define EVAL0(...) __VA_ARGS__
#define EVAL1(...) EVAL0(EVAL0(EVAL0(__VA_ARGS__)))
#define EVAL2(...) EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL3(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL4(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL(...)  EVAL4(EVAL4(EVAL4(__VA_ARGS__)))

#define MAP_END(...)
#define MAP_OUT
#define MAP_COMMA ,

#define MAP_GET_END2() 0, MAP_END
#define MAP_GET_END1(...) MAP_GET_END2
#define MAP_GET_END(...) MAP_GET_END1
#define MAP_NEXT0(test, next, ...) next MAP_OUT
#define MAP_NEXT1(test, next) MAP_NEXT0(test, next, 0)
#define MAP_NEXT(test, next)  MAP_NEXT1(MAP_GET_END test, next)

#define MAP0(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP1)(f, peek, __VA_ARGS__)
#define MAP1(f, x, peek, ...) f(x) MAP_NEXT(peek, MAP0)(f, peek, __VA_ARGS__)

#define MAP_LIST_NEXT1(test, next) MAP_NEXT0(test, MAP_COMMA next, 0)
#define MAP_LIST_NEXT(test, next)  MAP_LIST_NEXT1(MAP_GET_END test, next)

#define MAP_LIST0(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST1)(f, peek, __VA_ARGS__)
#define MAP_LIST1(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST0)(f, peek, __VA_ARGS__)

/**
 * Applies the function macro `f` to each of the remaining parameters.
 */
#define MAP(f, ...) EVAL(MAP1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

/**
 * Applies the function macro `f` to each of the remaining parameters and
 * inserts commas between the results.
 */
#define MAP_LIST(f, ...) EVAL(MAP_LIST1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

enum bool {
    false = 0,
    true = 1
};
typedef enum bool bool;

float random_range(float min, float max);


#endif /* macros_h */
