#include <iostream>
#include <string>
#include <chess/chess.hpp>
#include <mcts/node.hpp>
#include <mcts/policy.hpp>
#include <functional>
#include <memory>
// Usage: ./main MCTS_ITER MAX_MOVES print_time
// E.g. ./main 30 1000 1, ./main 30 50 1
int main(int argc, char* argv[])
{
    
    int MAX_MCTS_ITERATIONS{50};
    int MAX_MOVES{1000};
    bool print_time = 0;
    if(argc > 3) 
    {
        MAX_MCTS_ITERATIONS = std::stoi(argv[1]);
        MAX_MOVES = std::stoi(argv[2]);
        print_time = std::stoi(argv[3]);
    }

    // Initialize engine
    chess::init();
    // If we want to override rewards
    node::init(1.0, 0.0);

    // Initialize random generator
    static std::random_device random_device;
    static std::mt19937 generator(random_device());
    // Set rollout policy
    auto policy = std::bind(policy::rollout::random_rollout, std::placeholders::_1, std::placeholders::_2, generator, 10);

    // Initialize MCTS node
    chess::side player_side = chess::side::side_white;
    chess::side enemy_side = chess::side::side_black;
    chess::position start_p = chess::position::from_fen(chess::position::fen_start); // Or chess::position::from_fen("3K4/8/8/8/8/6R1/7R/3k4 w - - 0 1")
    std::shared_ptr<node::Node> main_node = std::make_shared<node::Node>(start_p, player_side, player_side);
    
    short moves{0};

    Timer timer{};
    double t_expanding{0};
    double t_traversing{0};
    double t_rollouting{0};
    double t_backpropping{0};
    // Main MCTS loop
    while(!main_node->is_over() && moves++ < MAX_MOVES)
    {
        main_node->expand();
        for(int i = 0 ; i < MAX_MCTS_ITERATIONS ; ++i)
        {
            timer.set_start();
            std::shared_ptr<node::Node> current_node = main_node->traverse();
            t_traversing += timer.get_time();
            if(current_node->is_over()) break;
            
            if(current_node->get_n() != 0)
            {
                timer.set_start();
                current_node->expand();
                t_expanding += timer.get_time();
                current_node = current_node->get_children().front();
            }
            timer.set_start();
            current_node->rollout(policy);
            t_rollouting += timer.get_time(true);
            current_node->backpropagate();
            t_backpropping += timer.get_time();
        }

        std::cout << "--- Node tree after search ---" << std::endl << main_node->to_string(0) << std::endl << std::endl;
        // MCTS move
        chess::move best_move = main_node->best_move();
        std::cout << "decided to make move " << best_move.to_lan() << ", total player moves: " << moves << '/' << MAX_MOVES << std::endl;
        chess::position new_state = main_node->get_state().copy_move(best_move);
        if(new_state.is_checkmate() || new_state.is_stalemate()) 
        {
            std::cout << "yup." << std::endl;
            break;
        }
        // CPU move
        std::vector<chess::move> cpu_moves{new_state.moves()};
        chess::move cpu_move = *random_element(cpu_moves.begin(), cpu_moves.end(), generator);
        new_state.make_move(cpu_move);
        main_node = std::make_shared<node::Node>(new_state, new_state.get_turn(), player_side, true, std::weak_ptr<node::Node>(), best_move);
    }
    
    std::cout << "done. Final state:\n" << main_node->get_state().pieces().to_string() << std::endl;
    std::cout << "statistics:\nstale mate" <<  main_node->get_state().is_stalemate()
    << "\ncheck mate" <<  main_node->get_state().is_checkmate();
    if(print_time)
    {
        std::cout 
        << "MCTS time report (in s): \ntime spent traversing: " << t_traversing 
        << "\ntime spent expanding: " << t_expanding 
        << "\ntime spent rollouting: " << t_rollouting 
        << "\ntime spent backpropping:" << t_backpropping;
    }
    return 0;
}