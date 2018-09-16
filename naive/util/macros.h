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

#define DEBUG 1

#define log_print(type, message, ...) printf("[" type "] " message "\n", ##__VA_ARGS__)

#define debug_print(message, ...) \
    do { if (DEBUG) log_print("DEBUG", message " (%s:%d)\n", ##__VA_ARGS__, __FILE__, __LINE__); } while (0)
#define log_info(message, ...) log_print("INFO", message, ##__VA_ARGS__)
#define log_ok(message, ...) log_print("OK", message, ##__VA_ARGS__)
#define log_error(message, ...) log_print("ERROR", message, ##__VA_ARGS__)

#define TYPE_INDEX 116 // "t" = 116 t_Vector, t_Matrix
#define IS(type, typeName) (*(char*)type != 0 && (*(*(char**)type)) == TYPE_INDEX && strcmp(*(char **)type, typeName) == 0)

float random_range(float min, float max);


#endif /* macros_h */
