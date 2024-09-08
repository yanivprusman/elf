# Compiler to use
CC = gcc

# Compiler flags
# CFLAGS = -Wall -Wextra -std=c99 -g
CFLAGS = -g -O0
 
# Name of the executable
TARGET = go

# Source files
SRCS = main.c 

# Object files (automatically generated from the source files)
OBJS = $(SRCS:.c=.o)

# Default target: Build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to build the object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build (remove the executable and object files)
# clean:
# 	rm -f $(OBJS) $(TARGET)
clean:
	del /Q $(subst /,\,$(OBJS)) $(subst /,\,$(TARGET))

# Phony targets (these targets do not correspond to actual files)
.PHONY: clean
