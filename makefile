FLAGS = -Wall -Werror

helpers_gcc:
	gcc helpers.c -c ${FLAGS}

shell_gcc:
	gcc shell.c -c ${FLAGS}

all:
	gcc shell.o helpers.o -o shell ${FLAGS}

make both:
	make helpers_gcc
	make shell_gcc

clean:
	rm -rf *.o
	rm shell