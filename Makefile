CFLAGS := -Wall -g -std=c11 -I. -D_POSIX_C_SOURCE=200809L
LDFLAGS := -g -T test_sec.lds
NASM := nasm

HDRS := $(wildcard *.h)
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

TARGET := shlang-cc

test: CFLAGS += -DTEST
test: Makefile $(TARGET)
	./shlang-cc

all: Makefile $(TARGET)

$(TARGET): $(HDRS) $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -lcunit -o $@

%.o:%.s
	$(NASM) $< -f elf64 -o $@

clean:
	rm -rf *.o $(TARGET)

.PHONY: all test clean
