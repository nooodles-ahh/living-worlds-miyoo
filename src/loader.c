#include "loader.h"
#include <stdio.h>
#include <stdlib.h>

char *read_file(const char *filename, size_t *size)
{
	FILE *file = fopen(filename, "r");
	char *code;

	if (file == NULL)
	{
		printf("Failed to read \"%s\"\n", filename);
		return NULL; // could not open file
	}
	fseek(file, 0L, SEEK_END);
	long fsize = ftell(file);

	fseek(file, 0, SEEK_SET);
	code = malloc(fsize + 1);

	fread(code, fsize, 1, file);
	code[fsize] = '\0';

	fclose(file);

	*size = fsize;
	return code;
}

struct json_value_s *read_json(const char *filename)
{
	size_t size;
	char *file = read_file(filename, &size);
	struct json_value_s *json = json_parse(file, size);
	free(file);
	return json;
}