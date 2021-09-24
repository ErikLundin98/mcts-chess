#include <iostream>
#include <string>
#include <chess/chess.hpp>
#include <mcts/node.hpp>


int main(int argc, char* argv[])
{

    // Initialize engine
    chess::init();
    // Initialize random generator
    static std::random_device random_device;
    static std::mt19937 generator(random_device());
    // Initialize MCTS node
    chess::side player_side = chess::side::side_white;
    chess::side enemy_side = chess::side::side_black;
    chess::position start_p = chess::position::from_fen(chess::position::fen_start);
    Node main_node{start_p, player_side, player_side, true, nullptr, chess::move()};
    std::cout << "Initialized engine and monte carlo node";

    //while(!main_node.)

    std::cout << "done.";

    return 0;
}