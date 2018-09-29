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

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define FIRST_ARG_(N, ...) N
#define FIRST_ARG(args) FIRST_ARG_ args

#define log_print(type, message, ...) printf("[" type "] " message "\n", ##__VA_ARGS__)
#define debug_print(type, message, ...) log_print(type, message " (%s:%d)\n", ##__VA_ARGS__, __FILENAME__, __LINE__)

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_info(message, ...) log_print("INFO", message, ##__VA_ARGS__)
#define log_ok(message, ...) log_print("OK", message, ##__VA_ARGS__)
#define log_error(message, ...) debug_print("ERROR", "[errno: %s] " message, clean_errno(), ##__VA_ARGS__)
#define log_warning(message, ...) log_print("WARNING", "[errno: %s] " message, clean_errno(), ##__VA_ARGS__)
#define push_error(message, ...) { log_error(message, ##__VA_ARGS__); errno = 0; goto error; }

#define check(expression, message, ...) if(!(expression)) push_error(message, ##__VA_ARGS__)
#define check_memory(variable, ...) check((variable), "Out of memory.")
#define check_debug(expression, message, ...) if(!(expression)) { debug(message, ##__VA_ARGS__); }
#define sentinel(message, ...) push_error(message, ##__VA_ARGS__)


// Weird predefined struct type cheker
#define TYPE_INDEX 116 // "t" = 116 t_Vector, t_Matrix
#define IS(type, typeName) (*(char*)type != 0 && (*(*(char**)type)) == TYPE_INDEX && strcmp(*(char **)type, typeName) == 0)

float random_range(float min, float max);


#endif /* macros_h */
