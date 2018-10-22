//
//  router.c
//  math
//
//  Created by Alexandr Kondratyev on 07/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#include "router.h"

static void                 router_any(neural_network *network, size_t layer);
static void                 __router_create_synapse(neural_cell *from_cell, neural_cell *to_cell);

/* Library structure */
const struct router_library Router = {
    .any = router_any
};

static
void
router_any(neural_network *network, size_t layer) {
    size_t layer_dimension = network->resolution.dimensions[layer];
    check(layer_dimension > 0, "Layer size is NULL");
    
    for(size_t position = 0; position < layer_dimension; position++) {
        check(network->resolution.size > position, "Position is outside network size");
        neural_cell *cell = &NEURON(network, layer, position);
        neuron_ccheck(cell, "Cell for routing is broken");
        
        if(network->resolution.layers > layer + 1) {
            check(network->resolution.dimensions[layer + 1], "Next layer dimension is zero");

            cell->axon = Network.get.layer(network, layer + 1);
            check_memory(cell->axon);
            
            free(cell->reverse);
            cell->reverse = calloc(network->resolution.dimensions[layer + 1], sizeof(enum bool));
            check_memory(cell->reverse);
            
            for(size_t terminal_index = 0; cell->axon[terminal_index]; terminal_index++) {
                __router_create_synapse(cell, cell->axon[terminal_index]);
            }
        }
    }

error:
    return;
}

static
void
__router_create_synapse(neural_cell *from_cell, neural_cell *to_cell) {
    neuron_ccheck(from_cell, "Broken source cell");
    neuron_ccheck(to_cell, "Broken destination cell");

    size_t synapse_index = 0;
    
    do {
        if(to_cell->synapse[synapse_index] == NULL) {
            to_cell->synapse = realloc(to_cell->synapse, (synapse_index + 2) * sizeof(neural_cell*));
            check_memory(to_cell->synapse);
            
            to_cell->synapse[synapse_index] = from_cell;
            neuron_ccheck(to_cell->synapse[synapse_index], "Broken cell in synapse terminal");
            to_cell->synapse[++synapse_index] = NULL;
        }
    } while(to_cell->synapse[synapse_index++]);
    synapse_index--;
    
    to_cell->impulse = realloc(to_cell->impulse, synapse_index * sizeof(enum bool));
    check(synapse_index - to_cell->body.signal->rows >= 0, "New size may be only greater or equal");
    memset(to_cell->impulse + to_cell->body.signal->rows, 0, (synapse_index - to_cell->body.signal->rows) * sizeof(enum bool));
    
    Matrix.reshape(to_cell->body.signal, synapse_index, to_cell->body.signal->columns);
    Neuron.weight.init(&to_cell->body);
    matrix_check(to_cell->body.signal);

    check(synapse_index == to_cell->body.signal->rows == to_cell->body.weight->rows, "Signal and weight dimesion not proper");
    check(to_cell->synapse[synapse_index] == NULL, "Last synapse pointer isn't NULL");

    return;
error:
    return;
}

