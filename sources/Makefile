CFLAGS=-ggdb -Wall 

all: main

bag.o: bag.h bag.c 
	gcc -c $(CFLAGS) bag.c

case.o: case.h case.c
	gcc -c $(CFLAGS) case.c

tray.o: tray.h tray.c case.h player.h
	gcc -c $(CFLAGS) tray.c

player.o: player.h player.c bag.h
	gcc -c $(CFLAGS) player.c

actions.o: actions.h actions.c tray.h player.h
	gcc -c $(CFLAGS) actions.c

main.o: main.c actions.h bag.h case.h tray.h player.h actions.h
	gcc -c $(CFLAGS) main.c

main: main.o actions.o player.o tray.o bag.o case.o
	gcc -o main $(CFLAGS) main.o actions.o player.o tray.o bag.o case.o

clean: 
	rm -f *.o exo1_2_main test_affichage test_intersect comparer_intersect_main comparer_viaMinimize_main trier