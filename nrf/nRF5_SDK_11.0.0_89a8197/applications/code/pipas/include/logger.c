#include "logger.h"
#include <stdio.h>

void logger_create(logger_t* logger)
{
	logger = malloc(sizeof(logger_t));
	logger_reset(logger);
}

void logger_add_line(logger_t* logger, char* s)
{
	
	if(logger->current_position < LOGGER_MAX)
	{
		memcpy(&logger->buffer[logger->current_position * LOGGER_CHUNK_SIZE], s, LOGGER_CHUNK_SIZE);
		logger->current_position = logger->current_position + 1;
	}
}

void logger_reset(logger_t* logger)
{
	logger->current_position = 0;
}

void logger_next(logger_t* logger, char* n)
{
	memcpy(n, &logger->buffer[logger->current_position * LOGGER_CHUNK_SIZE], LOGGER_CHUNK_SIZE);
	if(logger->current_position < LOGGER_MAX)
	{
		logger->current_position = logger->current_position + 1;
	}
}