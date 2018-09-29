//
//  mnist.h
//  naive
//
//  Created by Alexandr Kondratyev on 15/09/2018.
//  Copyright © 2018 alexander. All rights reserved.
//

#ifndef mnist_h
#define mnist_h

#include <stdio.h>
#include "../data/set.h"

struct mnist_label_file {
    int mangic_number;
    int items;
    short *labels;
};

struct mnist_image {
    short *pixels[28*28];
};

struct mnist_image_file {
    int magic_number;
    int items;
    int width;
    int height;
    
    struct mnist_image images;
};

struct mnist_data {
    struct mnist_image_file image_file;
    struct mnist_label_file label_file;
    
    matrix *features;
    matrix *target;
};



#endif /* mnist_h */
