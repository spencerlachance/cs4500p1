build:
	g++ -pthread -g -std=c++11 test/test_dataframe.cpp
	wget https://raw.githubusercontent.com/spencerlachance/cs4500datafile/master/datafile.zip
	mkdir data
	mv datafile.zip data
	unzip data/datafile.zip -d data

run:
	./a.out -f data/datafile.txt -len 10000000

valgrind:
	valgrind --leak-check=full ./a.out -f data/datafile.txt -len 1000000

clean:
	rm a.out
	rm -r data