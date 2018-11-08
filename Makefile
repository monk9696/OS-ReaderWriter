## Compiler
CC = gcc

## Compiler Flags
## -g			Debugging enable
## -Wall		Shows all warnings
## -std=gnu99	For loop variables
## -lpthread    For thread support
## -lrt         Realtime support
CCFLAGS  = -g -Wall -std=gnu99 -lpthread -lrt

## Build target
TARGET = readerWriter

all: $(TARGET)

$(TARGET): Writer.c
	$(CC) $(CCFLAGS) -o $(TARGET) Writer.c

clean:
	$(RM) $(TARGET)
