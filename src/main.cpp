#include <chess/chess.hpp>
#include <mcts/node.hpp>
#include <mcts/policy.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>

// Usage: ./main MCTS_ITER <config filename>

int main(int argc, char* argv[])
{
    std::string config_file_name{"config.txt"};
    if(argc > 1) config_file_name = argv[1];
    std::unordered_map<std::string, int> dict = parse_config(config_file_name);
    int MAX_MOVES = dict["MAX_MOVES"];
    int MAX_MCTS_ITERATIONS = dict["MAX_MCTS_ITERATIONS"];
    int PRINT_TIME = dict["PRINT_TIME"];
    int ROLLOUT_SIMULATIONS = dict["ROLLOUT_SIMULATIONS"];
    int WIN_SCORE = dict["WIN_SCORE"];
    int DRAW_SCORE = dict["DRAW_SCORE"];
    int PRINT_DEPTH = dict["PRINT_DEPTH"];

    // Initialize engine
    chess::init();
    // If we want to override rewards
    node::init(WIN_SCORE, DRAW_SCORE);

    // Initialize random generator
    static std::random_device random_device;
    static std::mt19937 generator(random_device());
    // Set rollout policy
    auto policy = std::bind(policy::rollout::random_rollout, std::placeholders::_1, std::placeholders::_2, generator, ROLLOUT_SIMULATIONS);

    // Initialize MCTS node
    chess::side player_side = chess::side::side_white;
    chess::side enemy_side = chess::side::side_black;
    chess::position start_p = chess::position::from_fen(chess::position::fen_start); // Or chess::position::from_fen("3K4/8/8/8/8/6R1/7R/3k4 w - - 0 1")
    // TODO: Make working with nodes easier for the user
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

        std::cout << "--- Node tree after search ---" << std::endl << main_node->to_string(PRINT_DEPTH) << std::endl << std::endl;
        // MCTS move
        chess::move best_move = main_node->best_move();
        std::cout << "decided to make move " << best_move.to_lan() << ", total player moves: " << moves << '/' << MAX_MOVES << std::endl;
        chess::position new_state = main_node->get_state().copy_move(best_move);
        if(new_state.is_checkmate() || new_state.is_stalemate()) 
        {
            // Break if game over
            main_node = std::make_shared<node::Node>(new_state, new_state.get_turn(), player_side, true, std::weak_ptr<node::Node>(), best_move);
            break;
        }
        // CPU move
        std::vector<chess::move> cpu_moves{new_state.moves()};
        chess::move cpu_move = *random_element(cpu_moves.begin(), cpu_moves.end(), generator);
        new_state.make_move(cpu_move);
        main_node = std::make_shared<node::Node>(new_state, new_state.get_turn(), player_side, true, std::weak_ptr<node::Node>(), best_move);
    }
    
    std::cout << "Game over. Final state:\n" << main_node->get_state().pieces().to_string() << std::endl;
    
    if(PRINT_TIME)
    {
        std::cout 
        << "MCTS time report (in s): \ntime spent traversing: " << t_traversing 
        << "\ntime spent expanding: " << t_expanding 
        << "\ntime spent rollouting: " << t_rollouting 
        << "\ntime spent backpropping:" << t_backpropping;
    }
    return 0;
}