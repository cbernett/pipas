#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define LOGGER_MAX 100
#define LOGGER_CHUNK_SIZE 200

typedef struct logger_t
{
	uint16_t current_position;
	char buffer[LOGGER_MAX * LOGGER_CHUNK_SIZE];
} logger_t;

void logger_create(logger_t*);
void logger_add_line(logger_t*, char*);
void logger_reset(logger_t*);
uint8_t logger_end(logger_t);
void logger_next(logger_t*, char* n);