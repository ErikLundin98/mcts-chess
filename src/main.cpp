#include <iostream>
#include <string>
#include <chess/chess.hpp>
#include <mcts/node.hpp>


int main(int argc, char* argv[])
{

    // Initialize engine
    chess::init();
    std::cout << "Engine initialized";
    // Initialize random generator
    static std::random_device random_device;
    static std::mt19937 generator(random_device());

    chess::side player_side = chess::side::side_white;
    chess::side enemy_side = chess::side::side_black;
    // Set start position
    chess::position start_p = chess::position::from_fen(chess::position::fen_start);

    Node start_node{start_p, player_side, player_side, true, nullptr, chess::move()};

    start_node.rollout(generator);

    std::cout << "done.";

    return 0;
}