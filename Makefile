.PHONY: word

build:
	g++ -pthread -g -std=c++11 -o dataf test/test_dataframe.cpp
	g++ -pthread -g -std=c++11 -o serial test/test_serialization.cpp
	g++ -pthread -g -std=c++11 -o trivial test/trivial.cpp
	g++ -pthread -g -std=c++11 -o demo test/demo.cpp
	g++ -pthread -g -std=c++11 -o word test/word_count.cpp
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

clean:
	rm dataf serial trivial demo word data/datafile.*

word:
	g++ -pthread -g -std=c++11 -o word test/word_count.cpp
	./word -i 0 -n 3 -f data/100k.txt &
	./word -i 1 -n 3 -f data/100k.txt &
	./word -i 2 -n 3 -f data/100k.txt
	rm word