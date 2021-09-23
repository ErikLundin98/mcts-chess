#pragma once

#include <chess/chess.hpp>
#include <mcts/misc.hpp>
#include <array>
#include <vector>
#include <iostream>
// TODO: Separate
#include <iterator>

class Node
{
    public:
        Node(chess::position state, Node* parent=nullptr) : state{state}, parent{parent}
        {}
        inline std::vector<Node*> get_children() const {
            return this->children;
        }
        template<typename RNG>
        inline void rollout(RNG generator)
        {
            chess::position rollout_state{state};
            // by default, do random rollouts
            
            while(!rollout_state.is_checkmate() && !rollout_state.is_stalemate())
            {
                std::vector<chess::move> available_moves{rollout_state.moves()};
                chess::move random_choice = *random_element(std::begin(available_moves), std::end(available_moves), generator);
                rollout_state.make_move(random_choice);
            }
            std::cout << "loser: " << rollout_state.get_turn() << std::endl;
        }
            
        
    private:
        chess::position state;
        Node* parent;
        std::vector<Node*> children;
};