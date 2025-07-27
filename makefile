LIB=-lraylib -lGL -lm -lpthread -ldl -lrt -lX11
CC=gcc

all: clean build run

clean:
	rm -f uitest

build:
	${CC} ./*.c -o uitest -I . ${LIB}

run:
	./uitest
test:
	${CC} -g ./*.c -o uitest -I . ${LIB}
	gdb ./uitest
