SRC=main.c calc.c draw.c font.c gcode.c glyph.c image.c conf.c

all: 
	gcc -o cnc `pkg-config --libs --cflags glib-2.0 gtk+-2.0` -lm $(SRC)
