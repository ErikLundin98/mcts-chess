#include <chess/chess.hpp>
#include <mcts/node.hpp>
#include <mcts/policy.hpp>
#include <iostream>
#include <functional>

int main()
{
    chess::init();
    chess::position start_p = chess::position::from_fen(chess::position::fen_start);
    chess::side player_side = chess::side::side_white;
    chess::side enemy_side = chess::side::side_black;
    static std::random_device random_device;
    static std::mt19937 generator(random_device());

    
    auto policy = policy::rollout::bad_rollout; //std::bind(random_rollout<1>, std::placeholders::_1, std::placeholders::_2, generator);

    node::Node main_node = node::Node{start_p, player_side, player_side};
    main_node.expand();
    node::Node* second_child = main_node.get_children().front();
    second_child->rollout(policy);
    std::cout << "done";
}