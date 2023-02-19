#ifndef LOADER_H
#define LOADER_H
#include "json.h"

char *read_file(const char *filename, size_t *size);
struct json_value_s *read_json(const char *filename);
#endif