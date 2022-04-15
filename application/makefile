COMPILER = gcc
EXE_NAME = main
PORT = /dev/ttyS1

default:
	$(COMPILER) -c linklayer.c
	$(COMPILER) main.c linklayer.o -o $(EXE_NAME)
	@#$(COMPILER) -w main.c ../protocol/*.o -o $(EXE_NAME)

tx:
	./$(EXE_NAME) $(PORT) tx 

rx:
	./$(EXE_NAME) $(PORT) rx 
