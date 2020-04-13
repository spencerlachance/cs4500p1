.PHONY: word linus demo serial

build:
	g++ -pthread -g -std=c++11 -o dataf test/test_dataframe.cpp
	g++ -pthread -g -std=c++11 -o serial test/test_serialization.cpp
	g++ -pthread -g -std=c++11 -o trivial test/trivial.cpp
	g++ -pthread -g -std=c++11 -o demo test/demo.cpp
	g++ -pthread -g -std=c++11 -o word test/word_count.cpp
	g++ -pthread -g -std=c++11 -o linus test/linus.cpp
	wget https://raw.githubusercontent.com/spencerlachance/cs4500datafile/master/datafile.zip
	mv datafile.zip data
	unzip data/datafile.zip -d data

run:
	./dataf -f data/datafile.txt -len 1000000
	./serial
	./trivial
	./demo -idx 1 &
	./demo -idx 2 &
	./demo -idx 0
	./word -i 1 -n 3 -f data/100k.txt &
	./word -i 2 -n 3 -f data/100k.txt &
	./word -i 0 -n 3 -f data/100k.txt
	./linus -i 1 -n 4 &
	./linus -i 2 -n 4 &
	./linus -i 3 -n 4 &
	./linus -i 0 -n 4 -l 1000000

valgrind:
	valgrind --leak-check=full ./dataf -f data/datafile.txt -len 100000
	valgrind --leak-check=full ./serial
	valgrind --leak-check=full ./trivial -v
	valgrind --leak-check=full ./demo -v -idx 1 &
	valgrind --leak-check=full ./demo -v -idx 2 &
	valgrind --leak-check=full ./demo -v -idx 0
	valgrind --leak-check=full ./word -i 1 -n 3 -f data/100k.txt &
	valgrind --leak-check=full ./word -i 2 -n 3 -f data/100k.txt &
	valgrind --leak-check=full ./word -i 0 -n 3 -f data/100k.txt
	valgrind --leak-check=full ./linus -i 1 -n 4 &
	valgrind --leak-check=full ./linus -i 2 -n 4 &
	valgrind --leak-check=full ./linus -i 3 -n 4 &
	valgrind --leak-check=full ./linus -i 0 -n 4 -l 100000

clean:
	rm dataf serial trivial demo word linus data/datafile.*

serial:
	g++ -pthread -g -std=c++11 -o serial test/test_serialization.cpp
	./serial
	rm serial

demo:
	g++ -pthread -g -std=c++11 -o demo test/demo.cpp
	./demo -idx 1 &
	./demo -idx 2 &
	./demo -idx 0
	rm demo

word:
	g++ -pthread -g -std=c++11 -o word test/word_count.cpp
	./word -i 0 -n 3 -f data/100k.txt &
	./word -i 1 -n 3 -f data/100k.txt &
	./word -i 2 -n 3 -f data/100k.txt
	rm word

linus:
	g++ -pthread -g -std=c++11 -o linus test/linus.cpp
	./linus -i 1 -n 4 &
	./linus -i 2 -n 4 &
	./linus -i 3 -n 4 &
	./linus -i 0 -n 4 -l 1000000
	rm linus