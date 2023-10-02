all: run

run:
	g++ -std=c++11 server.cpp -o server.exe -lpthread -lssl -lcrypto
	g++ -std=c++11 client.cpp -o client.exe -lpthread -lssl -lcrypto
	g++ -std=c++11 test.cpp -o test.exe -lpthread -lssl -lcrypto

clean:
	rm -rf *.exe
	rm -r Angel/
	rm -r Diana/
	rm -r Jose/