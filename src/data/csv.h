#pragma once

#define CSV_ERR_LONGLINE 0
#define CSV_ERR_NO_MEMORY 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSV_TYPE "t_CSV"

typedef struct {
    char *type;
    
    size_t rows;
    size_t columns;
    char **fields;
    char ***values;
} csv;

struct csv_library {
    struct {
        csv *      (*file)(char *filename);
    } from;

    void           (*delete)(csv *instance);
    struct {
        char **    (*row)(csv *instance, size_t row);
        char **    (*column)(csv *instance, size_t column);
        char **    (*field)(csv *instance, char *field_name);
    } get;
};

extern struct csv_library Csv;
