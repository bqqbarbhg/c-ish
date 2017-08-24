#pragma once

#include <stdint.h>

struct symbol { uint32_t index; };

symbol intern(const char *str);

