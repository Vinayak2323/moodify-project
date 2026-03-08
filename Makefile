# Compiler
CC = gcc
CFLAGS = -Wall

# Object files
OBJ = main.o file_io.o features.o rules.o database.o auth.o search_sort.o utils.o http_server.o

# Executable name
TARGET = run

# Default rule
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) -pthread

# Individual compilation rules
main.o: main.c file_io.h features.h rules.h database.h auth.h search_sort.h
	$(CC) $(CFLAGS) -c main.c

file_io.o: file_io.c file_io.h features.h
	$(CC) $(CFLAGS) -c file_io.c

features.o: features.c features.h
	$(CC) $(CFLAGS) -c features.c

rules.o: rules.c rules.h features.h
	$(CC) $(CFLAGS) -c rules.c

database.o: database.c database.h features.h rules.h
	$(CC) $(CFLAGS) -c database.c

auth.o: auth.c auth.h database.h features.h
	$(CC) $(CFLAGS) -c auth.c

search_sort.o: search_sort.c search_sort.h features.h database.h
	$(CC) $(CFLAGS) -c search_sort.c

http_server.o: http_server.c http_server.h database.h file_io.h
	$(CC) $(CFLAGS) -c http_server.c

# Clean build files
clean:
	del *.o $(TARGET).exe 2>nul || rm -f *.o $(TARGET)
