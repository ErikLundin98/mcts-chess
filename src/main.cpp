#include <iostream>
#include <string>
#include <chess/chess.hpp>
#include <mcts/node.hpp>


int main(int argc, char* argv[])
{
    int MAX_MCTS_ITERATIONS{50};
    if(argc > 1) 
    {
        MAX_MCTS_ITERATIONS = std::stoi(argv[1]);
    }
    
    // Initialize engine
    chess::init();
    // Initialize random generator
    static std::random_device random_device;
    static std::mt19937 generator(random_device());
    // Initialize MCTS node
    chess::side player_side = chess::side::side_white;
    chess::side enemy_side = chess::side::side_black;
    // start position: chess::position::fen_start 
    // position with check mate in 1 turn for white: chess::position::from_fen("3K4/8/8/8/8/6R1/7R/3k4 w - - 0 1")
    chess::position start_p = chess::position::from_fen(chess::position::fen_start);
    Node main_node{start_p, player_side, player_side, true, nullptr, chess::move()};
    std::cout << "Initialized engine and monte carlo node" << std::endl;
    std::cout << "Using amount of mcts iterations=" << MAX_MCTS_ITERATIONS << std::endl;
    short moves{0};
    int max_moves{1000};
    while(!main_node.is_over() && moves++ < max_moves)
    {
        // First, expand the first node
        main_node.expand();
        
        for(int i = 0 ; i < MAX_MCTS_ITERATIONS ; ++i)
        {
            // Find best node to explore by traversing the tree
            Node* current_node{main_node.traverse()};
            
            // Check if it is a terminal node
            if(current_node->is_over()) break;
            
            // If still in loop current_node points to non-terminal leaf node
            if(current_node->get_n() != 0)  // Expand
            {
                current_node->expand();
               
                current_node = current_node->get_children().front();
                
            }
            // Rollout and backpropagate
            current_node->rollout(generator);
            current_node->backpropagate();
        }
        std::cout << "--- Node tree after search ---" << std::endl << main_node.to_string(0) << std::endl << std::endl;
        // Make our best move to a new state
        chess::move best_move = main_node.best_move();
        std::cout << "decided to make move " << best_move.to_lan() << ", total player moves: " << moves << std::endl;
        chess::position new_state = main_node.get_state().copy_move(best_move);
        // Check if we win
        if(new_state.is_checkmate() || new_state.is_stalemate()) break;
        // Enemy move
        new_state.make_move(new_state.moves().front());
        main_node = Node(new_state, new_state.get_turn(), player_side, true, nullptr, chess::move());
        // Print current node tree
    }

    std::cout << "done. Final state:\n" << main_node.get_state().pieces().to_string();
    return 0;
}