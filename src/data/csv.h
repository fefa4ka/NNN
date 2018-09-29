#ifndef CSV_DOT_H_INCLUDE_GUARD
#define CSV_DOT_H_INCLUDE_GUARD

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

csv *csv_readfile(char *filename);
void csv_delete(csv *instance);
char **csv_row(csv *instance, size_t row);
char **csv_column(csv *instance, size_t column);
char **csv_field(csv *instance, char *field_name);

#endif

