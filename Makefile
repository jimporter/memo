.PHONY: clean test

test:
	clang++ -std=c++1y -Wall -Wextra -pedantic -Werror -lboost_program_options -I../mettle/include -o test/test_memoizer test/test_memoizer.cpp
	./test/test_memoizer --verbose --color

clean:
	rm test/test_memoizer
