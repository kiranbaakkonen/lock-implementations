counter: lock.c lock.h counter.c
	gcc -Werror -Wall -O3 counter.c lock.c paddedprim.c stopwatch.c statistics.c -o counter -lm -lpthread -g3

clean:
	rm counter
