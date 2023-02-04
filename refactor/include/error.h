#pragma once

#include "log.h"
#include <errno.h>

#ifndef DEBUG
#define DEBUG 1
#endif

#define push_error(message, ...) { log_error(message, ##__VA_ARGS__); goto error; }

#define CHECK(expression, message, ...) do { if(DEBUG && !(expression)) push_error(message, ##__VA_ARGS__) } while(0)
#define CHECK_MEMORY_LOG(variable, message, ...) CHECK((variable), "Out of memory. " message, ##__VA_ARGS__)
#define CHECK_MEMORY(variable) CHECK_MEMORY_LOG(variable, "")
#define check_debug(expression, message, ...) if(!(expression)) { debug(message, ##__VA_ARGS__); }
#define sentinel(message, ...) push_error(message, ##__VA_ARGS__)
