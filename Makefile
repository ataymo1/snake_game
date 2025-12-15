snake: snake.c
	gcc -o snake snake.c -lncurses

clean:
	rm -f snake

.PHONY: clean

