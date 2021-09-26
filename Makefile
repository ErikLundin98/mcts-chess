ifeq ($(OS),Windows_NT)
    RM = del /f .\output\*.o
	MKDIR = if not exist ".\output" mkdir "output"
else
    RM = rm -f simulator ./output/*.o
	MKDIR = mkdir -p output
endif


main: output/main.o # output/chess.o
	g++ -std=c++20 -O3 output/main.o -o main

output/main.o: src/main.cpp include/mcts/*
	$(MKDIR)
	g++ -std=c++20 -O3 -c src/main.cpp -o output/main.o -I "./include/libchess/include" -I "./include"

debug:
	g++ -g -std=c++20 -c src/main.cpp -o output/main_debug.o -I "./include/libchess/include" -I "./include"
clean:
	$(RM)
