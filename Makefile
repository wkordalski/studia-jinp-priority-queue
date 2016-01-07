CXX=clang++
FLAGS=-std=c++11 -g
# FLAGS=-std=c++1z -g

TESTS=test test_exceptions
TESTS_FB=test_fb_1 test_fb_2   

VALGRIND_OPTS=--leak-check=full --show-leak-kinds=all --suppressions=valgrind.suppressions 

tests: $(TESTS)

test:
	$(CXX) $(FLAGS) test.cc -o test

test_exceptions:
	$(CXX) $(FLAGS) test_exceptions.cc -o test_exceptions

test_fb_1: test_fb_1.cc priorityqueue.hh
	$(CXX) $(FLAGS) test_fb_1.cc -o test_fb_1

test_fb_2: test_fb_2.cc priorityqueue.hh
	$(CXX) $(FLAGS) test_fb_2.cc -o test_fb_2

valgrind:
	valgrind $(VALGRIND_OPTS) ./test
	valgrind $(VALGRIND_OPTS) ./test_exceptions

clean:
	rm -f $(TESTS)

