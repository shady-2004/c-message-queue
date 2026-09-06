CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -D_POSIX_C_SOURCE=200809L -Iinclude
LDFLAGS = -pthread

# Default build
all: test_queue

# In-memory queue test
test_queue: src/queue.c tests/test_queue.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Address & Undefined Behavior Sanitizer (catches leaks, buffer overflows)
asan: CFLAGS += -fsanitize=address,undefined -g
asan: clean test_queue

# Thread Sanitizer (critical for detecting data races and mutex misuse)
tsan: CFLAGS += -fsanitize=thread -g
tsan: clean test_queue

clean:
	rm -f test_queue *.o

.PHONY: all asan tsan clean