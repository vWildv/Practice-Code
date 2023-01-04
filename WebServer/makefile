CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server: main.cc ./src/*cc
	$(CXX) --std=c++11 main.cc src/*.cc -I ./include -o server $(CXXFLAGS) -lpthread -lmysqlclient

clean:
	rm  -r server