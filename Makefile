CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -Iinclude -D_DEFAULT_SOURCE
TARGET  = myls

SRCS    = src/main.c src/ls_args.c src/ls_entry.c src/ls_compare.c \
          src/ls_format.c src/ls_walk.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
