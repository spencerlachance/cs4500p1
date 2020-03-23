build:
	g++ -pthread -g -std=c++11 -o dataf test/test_dataframe.cpp
	g++ -pthread -g -std=c++11 -o serial test/test_serialization.cpp
	g++ -pthread -g -std=c++11 -o kvstore test/test_kvstore.cpp
	wget https://raw.githubusercontent.com/spencerlachance/cs4500datafile/master/datafile.zip
	mkdir data
	mv datafile.zip data
	unzip data/datafile.zip -d data

run:
	./dataf -f data/datafile.txt -len 10000000
	./serial
	./kvstore

valgrind:
	valgrind --leak-check=full ./dataf -f data/datafile.txt -len 1000000
	valgrind --leak-check=full ./serial
	valgrind --leak-check=full ./kvstore

clean:
	rm serial dataf kvstore
	rm -r data