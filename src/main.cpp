#include <iostream>
#include <string>
#include <chess/chess.hpp>
#include <node.hpp>


int main(int argc, char* argv[])
{

    // Initialize engine
    std::cout << "load engine";
    chess::init();
    std::cout << "done loading";
    // Set start position
    chess::position start_p = chess::position::from_fen(chess::position::fen_start);

    Node start_node{start_p};

    std::cout << "done.";

    return 0;
}