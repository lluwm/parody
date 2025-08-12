CXX = g++
CXXFLAGS = -Wall -g -Iinclude -std=c++14 -stdlib=libc++

TARGET = parody

OBJS = 	./src/parody.o \
		./src/index.o \
		./src/btree.o \
		./src/key.o \
		./src/node.o

all: $(TARGET)

parody: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)
	
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f */*.o $(TARGET)