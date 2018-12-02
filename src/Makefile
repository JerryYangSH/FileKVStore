CXX     	= g++
CXXFLAGS	= -std=c++11 -I../include
FLAGS		= -o
LDFLAGS 	= 
SRCS 		= $(shell find . -name "*.cpp")
TARGETS		= $(subst .cpp,.o,$(SRCS))
EXECUTABLE	= store


all: $(TARGETS)
	$(CXX) $(CXXFLAGS) -o $(EXECUTABLE) $(TARGETS)

.PHONY: clean 

clean:
	rm -f *.o $(EXECUTABLE)

