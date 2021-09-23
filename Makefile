ifeq ($(OS),Windows_NT)
    RM = del /f .\output\*.o
	MKDIR = if not exist ".\output" mkdir "output"
else
    RM = rm -f simulator ./output/*.o
	MKDIR = mkdir -p output
endif


main: output/main.o # output/chess.o
	g++ -std=c++20 output/main.o -o main

output/main.o: src/main.cpp
	$(MKDIR)
	g++ -std=c++20 -c src/main.cpp -o output/main.o -I "./include/libchess/include" -I "./include/mcts"

clean:
	$(RM)