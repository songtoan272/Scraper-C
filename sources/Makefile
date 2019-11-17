DIR=../bin
CFLAGS=-ggdb -Wall -g 
SOURCES=main.c configuration.c url.c parse.c
OBJECTS=$(SOURCES:.c=.o)

all: main

url.o: url.h url.c 
	gcc -o $(DIR)/$@ -c $(CFLAGS) url.c

configuration.o: configuration.h configuration.c
	gcc -o $(DIR)/$@  -c $(CFLAGS) configuration.c

parse.o: parse.h parse.c
	gcc -o $(DIR)/$@  -c $(CFLAGS) parse.c

main.o: main.c url.h configuration.h
	gcc -o $(DIR)/$@  -c $(CFLAGS) main.c 

main: $(OBJECTS)
	gcc -o $(DIR)/$@ $(CFLAGS) $(DIR)/*.o -lcurl

clean: 
	rm -f $(DIR)/*.o main