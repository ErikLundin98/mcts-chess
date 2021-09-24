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
        Node(chess::position state, chess::side side, chess::side player_side, bool is_start_node=false, Node* parent=nullptr, chess::move move=chess::move()) : 
        state{state}, 
        node_side{side},
        player_side{player_side},
        is_start_node{is_start_node},
        parent{parent},
        move{move}
        {
            this->children = {};
            this->t = 0;
            this->n = 0;
        }

        inline std::vector<Node*> get_children() const {
            return this->children;
        }
        template<typename RNG>
        inline void rollout(RNG generator)
        {
            chess::position rollout_state{state};
            // by default, do random rollouts
            short uneventful_timer = 0;
            while(!rollout_state.is_checkmate() && !rollout_state.is_stalemate() && uneventful_timer < 50)
            {
                std::vector<chess::move> available_moves{rollout_state.moves()};
                chess::move random_choice = *random_element(std::begin(available_moves), std::end(available_moves), generator);
                chess::undo undo = rollout_state.make_move(random_choice);
                uneventful_timer = undo.capture != chess::piece::piece_none ? 0 : uneventful_timer++;
            }

            bool is_player_turn = rollout_state.get_turn() == player_side;
            n++;
            if(rollout_state.is_checkmate())
            {
                t = is_player_turn ? -win_score : win_score; // Will be the losers side
            }
            else
            {
                t = draw_score;
            }
        }

        inline void backpropagate()
        {
            if(parent)
            {   
                parent->t += t;
                parent->n ++;

                if(!parent->is_start_node)
                {
                    parent->backpropagate();
                }
            }
        }

        inline void expand()
        {
            std::vector<chess::move> available_moves{state.moves()};

            for(auto it{available_moves.begin()} ; it != available_moves.end() ; ++it)
            {
                chess::position child_state = state;
                child_state.make_move(*it); // TODO: This will alternate between our and enemy move
                Node* new_child = new Node(child_state, child_state.get_turn(), player_side, false, this, *it);
                if(new_child->state.is_checkmate() || new_child->state.is_stalemate())
                {
                    if (new_child->state.is_checkmate())
                    {
                        new_child->t = new_child->state.get_turn() == player_side ? win_score : -win_score;
                    }
                    else
                    {
                        new_child->t = draw_score;
                    }
                    new_child->n = 1;
                    new_child->backpropagate();
                }
                children.push_back(new_child);
            }
        }

        inline Node* traverse()
        {
            return nullptr;
        }

        constexpr static double const win_score = 1;
        constexpr static double const draw_score = 0;

    protected:

        chess::position state;
        chess::side node_side;
        chess::side player_side;
        chess::move move;
        bool is_start_node;
        bool is_terminal_node = false;
        Node* parent;
        std::vector<Node*> children;
        double t;
        int n;
};