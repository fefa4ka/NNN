//
//  set.c
//  naive
//
//  Created by Alexandr Kondratyev on 14/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "set.h"

static data_set     data_from_csv(char *filename, char** fields, char **target);
static void         data_delete(data_set *data);
static void         data_print(data_set *data);
const struct data_library Data = {
    .csv = data_from_csv,
    .print = data_print,
    .delete = data_delete
};


static
data_set
data_from_csv(char *filename, char** feature_labels, char **target_labels) {
    csv *data = csv_readfile(filename);
    
    if(feature_labels == NULL) {
        size_t index = 0;
        feature_labels = calloc(data->columns, sizeof(char*));

        while(data->fields[index]) {
            size_t target_label_index = 0;
            while(target_labels[target_label_index]) {
                if(strcmp(data->fields[index], target_labels[target_label_index]) != 0) {
                    feature_labels[index] = data->fields[index];
                }
                target_label_index++;
            }
            index++;
        }
        feature_labels[index++] = NULL;
        feature_labels = realloc(feature_labels, index * sizeof(char*));
    }
//
    matrix *all_data = Matrix.csv(data, data->fields);
    matrix *features = Matrix.csv(data, feature_labels);
    matrix *target = Matrix.csv(data, target_labels);

    data_set dataset = {
        .data = data,
        .features = {
            .labels = feature_labels,
            .values = features,
        },
        .target = {
            .label = target_labels,
            .values = target
        },
        .probability = Probability.from.matrix(all_data,
                                               data->fields)
    };
//
    Matrix.delete(all_data);
    return dataset;
}

static
void
data_delete(data_set *data) {
    Matrix.delete(data->features.values);
    Matrix.delete(data->target.values);
    
    
    free(data->features.labels);
    Probability.delete(&data->probability);
      csv_delete(data->data);
//    free(data);
}

static
void
data_print(data_set *data) {
    size_t field_index = 0;
    log_info("Data Set Information");
    log_info("Features:");
    while(data->features.labels[field_index]) {
        printf("\t%s", data->features.labels[field_index]);
        field_index++;
    }
    printf("\n");
    
    Matrix.print(data->features.values);
    
    log_info("Target: %s", data->target.label[0]);
    Matrix.print(data->target.values);
}

