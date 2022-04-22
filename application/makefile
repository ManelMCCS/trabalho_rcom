COMPILER = gcc
EXE_NAME = main
PORT = /dev/ttyS1
TEST = test

linklayer.o:
	$(COMPILER) -c linklayer.c

default:
	$(COMPILER) -c linklayer.c
	$(COMPILER) main.c linklayer.o -o $(EXE_NAME)
	$(COMPILER) -w main.c ../protocol/*.o -o $(EXE_NAME)
tx:
	./$(EXE_NAME) $(PORT) tx 
rx:
	./$(EXE_NAME) $(PORT) rx
test_file: linklayer.o
	$(COMPILER) -g $(TEST).c linklayer.o -o test_file
	rm linklayer.o

test_exe: test_file
	./test_file
	rm test_file
test_valgrind: test_file
	valgrind ./test_file
	rm test_file