CC = gcc
CFLAGS = -Wall -I./include
LIBS = -lGL -lGLU -lglut -lm -ljpeg
OUTPUT = $(notdir $(basename $(shell pwd)))

SRC = $(wildcard src/*.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

exec: $(OUTPUT)
	rm -f src/*.o
	
$(OUTPUT): $(OBJ)
	$(CC) $(CFLAGS) -o $(OUTPUT) $^ $(LIBS)

main.o: src/main.c include/dessiner.h
	$(CC) -c $(CFLAGS) $<

%.o: %.c %.h
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f $(OUTPUT) src/*.o


