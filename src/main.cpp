#include <iostream>
#include <string>
#include <chess/chess.hpp>
#include <mcts/node.hpp>

void print_state(Node const & node)
{
    chess::board board = node.get_state().pieces();
    for(int r = chess::rank_1 ; r <= chess::rank_8 ; ++r)
    {
        for(int f = chess::file_a ; f <= chess::file_h ; ++f) {
            chess::rank rank = static_cast<chess::rank>(r);
            chess::file file = static_cast<chess::file>(f);
            std::pair<chess::side, chess::piece> side_and_piece = board.get(chess::cat_coords(file, rank));
            std::cout << side_and_piece.first << side_and_piece.second;
            std::cout << chess::piece_to_san(side_and_piece.first, side_and_piece.second);
        }
        std::cout << std::endl;
    }
}

int main(int argc, char* argv[])
{

    static int MAX_MCTS_ITERATIONS = 20;
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
    std::cout << "Initialized engine and monte carlo node" << std::endl;
    
    short moves = 0;
    while(!main_node.is_over())
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

        // Make our best move to a new state
        chess::move best_move = main_node.best_move();
        chess::position new_state = main_node.get_state().copy_move(best_move);
        // Check if we win
        if(new_state.is_checkmate() || new_state.is_stalemate()) break;
        // Enemy move
        new_state.make_move(new_state.moves().front());
        main_node = Node(new_state, new_state.get_turn(), player_side, true, nullptr, chess::move());
        //print_state(main_node);
        std::cout << moves++ << std::endl;
    }

    std::cout << "done.";

    return 0;
}