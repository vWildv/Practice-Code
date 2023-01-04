release: test.cpp
	g++ -std=c++11 test.cpp src/*.cc -I./include -L./lib -lgtest -lpthread

debug: test.cpp
	g++ -DDEBUG -std=c++11 test.cpp src/*.cc -I./include -L./lib -lgtest -lpthread
