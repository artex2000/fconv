SRC=main.c

all: 
	gcc -o cnc `pkg-config --libs --cflags gtk+-2.0` -lm $(SRC)
