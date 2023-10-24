1: prog1 prog2 prog3
	gcc one_to_one.c -o one_to_one
	./one_to_one ./prog1 ./prog2 ./prog3

2: prog1 prog2 prog3
	gcc one_to_three.c -o one_to_three
	./one_to_three ./prog1 ./prog2 ./prog3

prog1:
	gcc prog1.c -o prog1

prog2:
	gcc prog2.c -o prog2

prog3:
	gcc prog3.c -o prog3

clean:
	rm -rf one_to_one one_to_three prog1 prog2 prog3
