build:
	g++ -pthread -g -std=c++11 -o dataf test/test_dataframe.cpp
	g++ -pthread -g -std=c++11 -o serial test/test_serialization.cpp
	g++ -pthread -g -std=c++11 -o trivial test/trivial.cpp
	g++ -pthread -g -std=c++11 -o demo test/demo.cpp
	wget https://raw.githubusercontent.com/spencerlachance/cs4500datafile/master/datafile.zip
	mkdir data
	mv datafile.zip data
	unzip data/datafile.zip -d data

run:
	./dataf -f data/datafile.txt -len 1000000
	./serial
	./trivial
	./demo -idx 1 &
	./demo -idx 2 &
	./demo -idx 0

valgrind:
	valgrind --leak-check=full ./dataf -f data/datafile.txt -len 10000
	valgrind --leak-check=full ./serial
	valgrind --leak-check=full ./trivial -v
	# Valgrind on Node 0
	./demo -v -idx 1 &
	./demo -v -idx 2 &
	valgrind --leak-check=full ./demo -v -idx 0
	# Valgrind on Node 1
	./demo -v -idx 0 &
	./demo -v -idx 2 &
	valgrind --leak-check=full ./demo -v -idx 1
	# Valgrind on Node 2
	./demo -v -idx 0 &
	./demo -v -idx 1 &
	valgrind --leak-check=full ./demo -v -idx 2

clean:
	rm dataf serial trivial demo
	rm -r data