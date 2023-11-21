all: compile run clean

compile: Bridge.c
	gcc -o Bridge Bridge.c -pthread

run: compile
	./Bridge

clean:
	rm -rf Bridge