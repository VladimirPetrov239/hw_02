.PHONY: all clean

FLAGS = -O2 -Wall -Wextra -std=c++11 -Isrc
TEST_FLAGS = $(FLAGS) -Itest
CXX = g++

TARGET = hw_02
TEST_TARGET = hw_02_test

all: $(TARGET) $(TEST_TARGET)

$(TARGET): obj/main.o obj/huffman.o src/main.cpp src/huffman.cpp
	$(CXX) $(FLAGS) obj/main.o obj/huffman.o -o hw_02

$(TEST_TARGET): obj/test.o obj/huffman.o test/test.cpp src/huffman.cpp
	$(CXX) $(TEST_FLAGS) obj/huffman.o obj/test.o -o hw_02_test

obj/huffman.o: src/huffman.cpp | obj
	$(CXX) $(FLAGS) -c -o $@ $<

obj/main.o: src/main.cpp | obj
	$(CXX) $(FLAGS) -c -o $@ $<

obj/test.o: test/test.cpp | obj
	$(CXX) $(TEST_FLAGS) -c -o $@ $<

obj:
	mkdir -p obj

clean:
	rm -rf obj
	rm -f $(TARGET)
	rm -f $(TEST_TARGET)
