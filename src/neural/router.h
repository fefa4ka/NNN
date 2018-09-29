//
//  router.h
//  math
//
//  Created by Alexandr Kondratyev on 07/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef router_h
#define router_h

#include <stdio.h>
#include "network.h"

struct router_library {
    void    (*any)(neural_network *network, size_t layer);
    void    (*near)(neural_network *network, size_t layer);
    void    (*one)(neural_network *network, size_t layer);
    void    (*reccurent)(neural_network *network, size_t layer);
    void    (*whole)(neural_network *network, size_t layer);
    void    (*random)(neural_network *network, size_t layer);
};

extern const struct router_library Router;

#endif /* router_h */

