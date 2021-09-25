#include <iostream>
#include <string>
#include <chess/chess.hpp>
#include <mcts/node.hpp>
#include <mcts/policy.hpp>
#include <functional>

int main(int argc, char* argv[])
{
    int MAX_MCTS_ITERATIONS{50};
    if(argc > 1) 
    {
        MAX_MCTS_ITERATIONS = std::stoi(argv[1]);
    }
    
    // Initialize engine
    chess::init();
    // If we want to override rewards
    node::init(1.0, 0.0);
    // Initialize random generator
    static std::random_device random_device;
    static std::mt19937 generator(random_device());

    // Set rollout policy
    auto policy = std::bind(random_rollout<10>, std::placeholders::_1, std::placeholders::_2, generator);

    // Initialize MCTS node
    chess::side player_side = chess::side::side_white;
    chess::side enemy_side = chess::side::side_black;

    // position with check mate in 1 turn for white: chess::position::from_fen("3K4/8/8/8/8/6R1/7R/3k4 w - - 0 1")
    chess::position start_p = chess::position::from_fen(chess::position::fen_start);
    
    node::Node main_node{start_p, player_side, player_side, true, nullptr, chess::move()};
    
    std::cout << "Initialized engine and monte carlo node" << std::endl;
    std::cout << "Using amount of mcts iterations=" << MAX_MCTS_ITERATIONS << std::endl;
    short moves{0};
    int max_moves{1000};
    while(!main_node.is_over() && moves++ < max_moves)
    {
        
        main_node.expand();
        for(int i = 0 ; i < MAX_MCTS_ITERATIONS ; ++i)
        {
            node::Node* current_node{main_node.traverse()};
            if(current_node->is_over()) break;
            
            if(current_node->get_n() != 0)
            {
                current_node->expand();
                current_node = current_node->get_children().front();
            }
            current_node->rollout(policy);
            current_node->backpropagate();
        }

        std::cout << "--- Node tree after search ---" << std::endl << main_node.to_string(0) << std::endl << std::endl;
        chess::move best_move = main_node.best_move();
        std::cout << "decided to make move " << best_move.to_lan() << ", total player moves: " << moves << std::endl;
        chess::position new_state = main_node.get_state().copy_move(best_move);
        if(new_state.is_checkmate() || new_state.is_stalemate()) break;
        // CPU move
        new_state.make_move(new_state.moves().front());
        main_node = node::Node{new_state, new_state.get_turn(), player_side, true, nullptr, chess::move()};
    }

    std::cout << "done. Final state:\n" << main_node.get_state().pieces().to_string();
    return 0;
}