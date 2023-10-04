CC=clang++
FLAGS=-std=c++20 -g -O0 -DUNIT_TEST
ASAN=-fsanitize=address -fsanitize-address-use-after-return=always -fsanitize-address-use-after-scope \
	 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=leak -fsanitize=undefined
SRC=../src

test_all: unit_tests.cpp $(SRC)/input_reader.cpp $(SRC)/transport_catalogue.cpp test_framework.cpp
	$(CC) $(FLAGS) -DALL $^ -o $@.out

test_all_asan: unit_tests.cpp $(SRC)/input_reader.cpp $(SRC)/transport_catalogue.cpp test_framework.cpp
	$(CC) $(FLAGS) $(ASAN) -DALL $^ -o $@.out

test_input_reader: unit_tests.cpp $(SRC)/input_reader.cpp test_framework.cpp
	$(CC) $(FLAGS) -DINPUT_READER $^ -o $@.out

test_transport_catalogue: unit_tests.cpp $(SRC)/transport_catalogue.cpp test_framework.cpp
	$(CC) $(FLAGS) -DTRANSPORT_CATALOGUE $^ -o $@.out

clean:
	rm *.out
