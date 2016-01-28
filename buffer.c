#include "buffer.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

struct input_buffer
{
    char* data;
    size_t size;
    size_t pos;
    bool external_data;
};

input_buffer_t* buffer_open(const char* path)
{
    if (!path) {
        return NULL;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        return NULL;
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }

    void* data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    if (data == NULL) {
        return NULL;
    }

    input_buffer_t* ib = (input_buffer_t*) calloc(1, sizeof(*ib));
    if (!ib) {
        munmap(data, st.st_size);
        return NULL;
    }

    ib->data = data;
    ib->size = st.st_size;
    ib->pos = 0;
    ib->external_data = false;

    return ib;
}

input_buffer_t* buffer_mem(void* data, size_t size)
{
    if (!data) {
        return NULL;
    }

    input_buffer_t* ib = (input_buffer_t*) calloc(1, sizeof(*ib));
    if (!ib) {
        return NULL;
    }

    ib->data = data;
    ib->size = size;
    ib->pos = 0;
    ib->external_data = true;

    return ib;
}

void buffer_close(input_buffer_t* ib)
{
    if (ib && !ib->external_data) {
        munmap(ib->data, ib->size);
    }

    free(ib);
}

#if 0
const char* buffer_getline(input_buffer_t* ib)
{
    if (!ib) {
        return NULL;
    }

    if (ib->line == NULL) {
        return NULL;
    }

    const char* next = ib->line;
    while (*next != '\n' && *next != '\0') {
        ++next;
    }

    ptrdiff_t len = next - ib->line;
    const char* line = strndup(ib->line, len);
    if (!line) {
        return NULL;
    }

    ib->line = (*next == '\0' ? NULL : next + 1);
    ib->pos = 0;

    return line;
}
#endif // 0

bool buffer_iseof(input_buffer_t* ib)
{
    return (ib->pos > ib->size);
}

char buffer_getchar(input_buffer_t* ib)
{
    if (!ib) {
        return EOF;
    }

    if (buffer_iseof(ib)) {
        return EOF;
    }

    return ib->data[ib->pos++];
}

size_t buffer_get_offset(input_buffer_t* ib)
{
    if (!ib) {
        return 0;
    }

    return ib->pos;
}

void buffer_set_offset(input_buffer_t* ib, size_t pos)
{
    if (ib) {
        ib->pos = pos;
    }
}

/////////////////////////////////////////////////////////////////////////////////

#if defined(TEST)
#include "test.h"

static void input_buffer_test(void)
{
    // TODO
}
TEST_ADD(input_buffer_test);

#endif

/////////////////////////////////////////////////////////////////////////////////
