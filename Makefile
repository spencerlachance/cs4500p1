build:
	g++ -pthread -g -std=c++11 test/test.cpp

run:
	./a.out -f data/datafile.txt

valgrind:
	valgrind --leak-check=full ./a.out -f data/datafile.txt

clean:
	rm a.out