# Makefile for compiling the server with cmark library

CC = gcc
CFLAGS = -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lcmark
TARGET = server
SRCS = server.c
OBJS = $(SRCS:.c=.o)

# Detect if Homebrew is installed in /opt/homebrew
ifeq ($(shell test -d /opt/homebrew && echo yes),yes)
    CFLAGS = -I/opt/homebrew/include
    LDFLAGS = -L/opt/homebrew/lib -lcmark
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean

