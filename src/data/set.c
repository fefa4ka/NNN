//
//  set.c
//  naive
//
//  Created by Alexandr Kondratyev on 14/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "set.h"

static data_set     data_from_matrix(matrix *features, matrix *target);
static data_set     data_from_csv(char *filename, char** fields, char **target);
static void         data_delete(data_set *data);
static void         data_print(data_set *data);
static csv *        data_shuffle(csv *data);
static data_set *   data_part(data_set *set, size_t pool_offset, size_t pool_size);
static data_set **  data_mini_batch(data_set *set, size_t batch_size);
static data_batch   data_split(data_set *set, size_t batch_size, size_t train, size_t validation, size_t test);
static matrix *     data_vector_to_binary_columns(vector *column);

const struct data_library Data = {
    .matrix = data_from_matrix,
    .csv = data_from_csv,
    .split = data_split,
    .print = data_print,
    .convert = {
        .vector_to_binary = data_vector_to_binary_columns,
    },
    .delete = data_delete
};


static
data_set
data_from_matrix(matrix *features, matrix *target) {
    size_t number_of_fields = features->columns + target->columns;
    char **fields = malloc(number_of_fields * sizeof(char*));
    char **feature_labels = malloc(features->columns * sizeof(char*));
    char **target_labels = malloc(target->columns * sizeof(char*));
    
    matrix *all_data = Matrix.transpose(Matrix.create(features->rows, number_of_fields));
    matrix *features_T = Matrix.transpose(Matrix.copy(features));
    matrix *target_T = Matrix.transpose(Matrix.copy(target));
    memcpy(all_data->vector->values, features_T->vector->values, features->vector->size * sizeof(float));
    memcpy(&all_data->vector->values[features_T->vector->size], target_T->vector->values, target->vector->size * sizeof(float));
    all_data = Matrix.transpose(all_data);
    Matrix.delete(features_T);
    Matrix.delete(target_T);

    for(size_t index = 0; index < number_of_fields; index++) {
        fields[index] = (char*)malloc(3 * sizeof(char));
        sprintf(fields[index], "c%d", (int)index);

        if(index >= features->columns) {
            target_labels[index - features->columns] = fields[index];
        } else {
            feature_labels[index] = fields[index];
        }
    }

    return (data_set){
        .data = {
            .values = all_data,
            .fields = fields,
        },
        
        .features = {
            .labels = feature_labels,
            .values = Matrix.copy(features),
        },
        .target = {
            .labels = target_labels,
            .values = Matrix.copy(target)
        },
        .probability = Probability.from.matrix(all_data,
                                               fields)
    };
}

static
data_set
data_from_csv(char *filename, char** feature_labels, char **target_labels) {
    csv *data = data_shuffle(csv_readfile(filename));
    char **_fields = malloc((data->columns + 1) * sizeof(char*));
    char **_target_labels = malloc(data->columns * sizeof(char*));
    size_t _target_labels_index = 0;
    for (size_t index = 0; index < data->columns; index++) {
        _fields[index] = strdup(data->fields[index]);
    }
    _fields[data->columns] = NULL;
    
    matrix *all_data = Matrix.csv(data, _fields);
    
    if(feature_labels == NULL) {
        size_t index = 0;
        feature_labels = calloc(all_data->columns, sizeof(char*));

        while(_fields[index]) {
            size_t target_label_index = 0;
            while(target_labels[target_label_index]) {
                if(strcmp(_fields[index], target_labels[target_label_index]) != 0) {
                    feature_labels[index] = _fields[index];
                } else {
                    _target_labels[_target_labels_index] = _fields[index];
                    _target_labels_index++;
                }
                target_label_index++;
            }
            
            index++;
        }
        feature_labels[index++] = NULL;
        feature_labels = realloc(feature_labels, index * sizeof(char*));
    }

    _target_labels = realloc(_target_labels, (_target_labels_index + 1) * sizeof(char *));
    matrix *features = Matrix.csv(data, feature_labels);
    matrix *target = Matrix.csv(data, target_labels);
    csv_delete(data);
    
    data_set dataset = {
        .data = {
            .fields = _fields
        },
        .features = {
            .labels = feature_labels,
            .values = features,
        },
        .target = {
            .labels = _target_labels,
            .values = target
        },
        .probability = Probability.from.matrix(all_data,
                                               _fields)
    };
    
    dataset.data.values = dataset.probability.samples;

    Matrix.delete(all_data);
    return dataset;
}

static
void
data_delete(data_set *data) {
    for(size_t index = 0; index < data->data.values->columns; index++) {
        free(data->data.fields[index++]);
    }
    Matrix.delete(data->data.values);
    Matrix.delete(data->target.values);
    Matrix.delete(data->features.values);
    free(data->features.labels);
    free(data->target.labels);
    free(data->data.fields);
    Probability.delete(&data->probability);
}


static
data_set *
data_part(data_set *set, size_t pool_offset, size_t pool_size) {
    if(pool_size == 0) {
        return NULL;
    }
    
    data_set *pool = malloc(sizeof(data_set));
    size_t number_of_features = set->features.values->columns;
    size_t number_of_targets = set->target.values->columns;
    size_t number_of_fields = number_of_features + number_of_targets;
    
    matrix *pool_data = Matrix.create(pool_size, number_of_fields);
    pool_data->vector->values = &set->data.values->vector->values[pool_offset];
    pool->probability = Probability.from.matrix(pool_data,
                                                set->data.fields);
    
    matrix *pool_features = Matrix.create(pool_size, number_of_features);
    pool_features->vector->values = set->features.values->vector->values;
    pool->features.labels = set->features.labels;
    pool->features.values = pool_features;
    
    matrix *pool_target = Matrix.create(pool_size, number_of_targets);
    pool_target->vector->values = &set->target.values->vector->values[pool_offset];
    pool->target.labels = set->target.labels;
    pool->target.values = pool_target;
    
    pool->data = set->data;
    
    return pool;
}

static
matrix *
data_vector_to_binary_columns(vector *column) {
    vector *uniq = Vector.prop.uniq(column);
    matrix *columns = Matrix.create(column->size, uniq->size);
    
    vector_foreach(column) {
        float value = VECTOR(column, index);
        MATRIX(columns, index, Vector.prop.index_of(uniq, value)) = 1.;
    }
    
    Vector.delete(uniq);
    
    return columns;
}

static
data_set **
data_mini_batch(data_set *set, size_t batch_size) {
    if(batch_size == 0) {
        data_set **mini = malloc(sizeof(data_set*));
        *mini = set;
        
        return mini;
    }
    
    size_t set_size = set->features.values->rows;
    size_t number_of_batches = set_size / batch_size;
    data_set **mini = malloc(number_of_batches * sizeof(data_set*));
    
    for(size_t pool_index = 0; pool_index < number_of_batches; pool_index++) {
        size_t pool_offset = batch_size * pool_index;
        
        mini[pool_index] = data_part(set, pool_offset, batch_size);
    }
    
    return mini;
}

static
data_batch
data_split(data_set *set, size_t batch_size, size_t train, size_t validation, size_t test) {
    size_t set_size = set->data.values->rows;
    
    size_t train_size = set_size * train / 100;
    size_t validation_size = set_size * validation / 100;
    size_t test_size = set_size * test / 100;
    size_t number_of_batches = 1;
    
    data_set *train_set = data_part(set, 0, train_size);
    
    if(batch_size) {
        number_of_batches = train_size / batch_size;
    }
    
    return (data_batch) {
        .size = batch_size || set_size,
        .count = number_of_batches,
        .train = train_set,
        .mini = data_mini_batch(train_set, batch_size),
        .validation = data_part(set, train_size, validation_size),
        .test = data_part(set, train_size + validation_size, test_size)
    };
}

static
csv *
data_shuffle(csv *data) {
    // Shuffle
    size_t set_size = data->rows;
    for(size_t index = 0; index < set_size; index++) {
        size_t shuffled = (size_t)random_range(0, set_size);
        
        if(shuffled == index) {
            if(index == set_size - 1) {
                if(index != 0) {
                    shuffled = index - 1;
                }
            } else {
                shuffled = index + 1;
            }
        }
        
        char **shuffled_row = data->values[index];
        data->values[index] = data->values[shuffled];
        data->values[shuffled] = shuffled_row;
    }
    
    return data;
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
    
    log_info("Target: %s", data->target.labels[0]);
    Matrix.print(data->target.values);
}

