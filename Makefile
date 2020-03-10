build:
	g++ -pthread -g -std=c++11 test/test.cpp

run:
	./a.out

valgrind:
	valgrind --leak-check=full ./a.out

clean:
	rm a.out