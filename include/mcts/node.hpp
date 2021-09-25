#pragma once

#include <chess/chess.hpp>
#include <mcts/misc.hpp>
#include <array>
#include <vector>
#include <iostream>
// TODO: Separate
#include <iterator>
#include <float.h>
#include <math.h>
#include <iterator>
#include <iostream>

class Node
{
public:
    Node(chess::position state, chess::side side, chess::side player_side, bool is_start_node = false, Node *parent = nullptr, chess::move move = chess::move())
        : state{state},
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
    ~Node() = default;

    inline std::vector<Node *> get_children() const
    {
        return this->children;
    }
    template <typename RNG>
    inline void rollout(RNG generator)
    {
        double accumulated_t{0};
        int n_rollouts{10};
        for (int i = 0; i < n_rollouts; ++i)
        {
            chess::position rollout_state{state};
            // by default, do random rollouts
            short uneventful_timer = 0;
            while (!rollout_state.is_checkmate() && !rollout_state.is_stalemate() && uneventful_timer < 50)
            {
                std::vector<chess::move> available_moves{rollout_state.moves()};
                chess::move random_choice = *random_element(std::begin(available_moves), std::end(available_moves), generator);
                chess::undo undo = rollout_state.make_move(random_choice);
                uneventful_timer = undo.capture != chess::piece::piece_none ? 0 : uneventful_timer + 1;
            }

            bool is_player_turn = rollout_state.get_turn() == player_side;
            n++;
            if (rollout_state.is_checkmate())
            {
                accumulated_t += is_player_turn ? -win_score : win_score; // Will be the losers side
            }
            else
            {
                accumulated_t += draw_score;
            }
        }

        t = accumulated_t / n_rollouts;
    }

    inline void backpropagate()
    {
        if (parent)
        {
            parent->t += t;
            parent->n++;

            if (!parent->is_start_node)
            {
                parent->backpropagate();
            }
        }
    }

    inline void expand()
    {   
        std::vector<chess::move> available_moves{state.moves()};
        for (auto it{available_moves.begin()}; it != available_moves.end(); ++it)
        {
            chess::position child_state = state.copy_move(*it); // TODO - plays random moves for both players
            Node *new_child = new Node(child_state, child_state.get_turn(), player_side, false, this, *it);
            if (new_child->state.is_checkmate() || new_child->state.is_stalemate())
            {
                if (new_child->state.is_checkmate())
                {
                    new_child->t = new_child->state.get_turn() == player_side ? -win_score : win_score;
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

    inline double UCB1()
    {
        if (n == 0 || (parent && parent->n==0))
        {
            return DBL_MAX;
        }
        else
        {
            double v_bar = t / n;
            int N = parent ? parent->n : 1;
            return v_bar + sqrt(2 * log(N) / n);
        }
    }
    // Function that determines next node to expand/rollout
    inline Node *traverse()
    {
        std::vector<double> UCB1_scores{};
        for (auto it{children.begin()}; it != children.end(); ++it)
        {
            if (!(*it)->is_terminal_node)
                UCB1_scores.push_back((*it)->UCB1());
        }
        if (UCB1_scores.size() == 0)
        {
            is_terminal_node = true;
            return parent ? parent : this;
        }

        Node *best_child = get_max_element<Node *>(children.begin(), UCB1_scores.begin(), UCB1_scores.end());

        if (best_child->children.size() > 0)
        {
            return best_child->children.front()->traverse();
        }
        else
        {
            return best_child;
        }
    }

    // Can be useful if we want to keep the tree from the previous iterations
    Node *best_child() const
    {
        std::vector<double> UCB1_scores{};
        for (auto it{children.begin()}; it != children.end(); ++it)
        {
            UCB1_scores.push_back((*it)->UCB1());
        }
        return get_max_element<Node *>(children.begin(), UCB1_scores.begin(), UCB1_scores.end());
    }
    // Useful for baseline mcts algorithm
    chess::move best_move() const
    {
        return best_child()->move;
    }

    chess::position get_state() const
    {
        return state;
    }
    bool is_over() const
    {
        return is_terminal_node || state.is_checkmate() || state.is_stalemate();
    }
    int get_n() const
    {
        return n;
    }
    // Print the main node and its children
    std::string to_string(int layers_left=1) const
    {
        std::string tree{};
        tree += state.pieces().to_string();

        if(layers_left > 0) {
            tree += '\n' + "---children depth " + std::to_string(layers_left) + " ---\n";
            for(auto child_ptr : children)
            {
                tree += child_ptr->to_string(layers_left-1) + '\n';
            }
       
        }
        return tree;
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
    Node *parent;
    std::vector<Node *> children;
    double t;
    int n;
};