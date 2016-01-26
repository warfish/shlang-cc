/*
 * Imput buffers
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct input_buffer input_buffer_t;

input_buffer_t* buffer_open(const char* path);

input_buffer_t* buffer_mem(void* data, size_t size);

const char* buffer_getline(input_buffer_t* b);

char buffer_getchar(input_buffer_t* b);

size_t buffer_get_offset(input_buffer_t* ib);

void buffer_set_offset(input_buffer_t* ib, size_t pos);

bool buffer_iseof(input_buffer_t* ib);

void buffer_close(input_buffer_t* b);