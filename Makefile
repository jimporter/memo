.PHONY: clean test

test:
	clang++ -std=c++1y -stdlib=libc++ -Wall -Wextra -pedantic -I../mettle/include -o test/test_all test/test_all.cpp
	./test/test_all --verbose --color

clean:
	rm test/test_all
