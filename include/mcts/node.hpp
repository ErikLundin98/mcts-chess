#pragma once

#include <chess/chess.hpp>
#include <mcts/misc.hpp>
#include <array>
#include <vector>
#include <iterator>
#include <float.h>
#include <math.h>
#include <iterator>
#include <iostream>
#include <functional>

namespace node
{

    class Node
    {
        public:
            // Used to create a node that is not a parent node
            Node(chess::position state, chess::side side, chess::side player_side, bool is_start_node, Node *parent, chess::move move)
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
            // Used to create a parent node
            Node(chess::position state, chess::side side, chess::side player_side) : Node(state, side, player_side, true, nullptr, chess::move()) {}
            ~Node() = default;

            inline std::vector<Node *> get_children() const
            {
                return this->children;
            }
            template<typename PolicyFunction>
            void rollout(PolicyFunction rollout_policy)
            {
                t = rollout_policy(state, player_side);
                n++;
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
                for (chess::move child_move : available_moves)
                {
                    chess::position child_state = state.copy_move(child_move); // TODO - plays random moves for both players
                    Node * new_child = new Node{child_state, child_state.get_turn(), player_side, false, this, child_move};
                    if (new_child->state.is_checkmate() || new_child->state.is_stalemate())
                    {
                        if (new_child->state.is_checkmate())
                        {
                            new_child->t = new_child->state.get_turn() == player_side ? -WIN_SCORE : WIN_SCORE;
                        }
                        else
                        {
                            new_child->t = DRAW_SCORE;
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
                for (Node* child : children)
                {
                    if (!child->is_terminal_node)
                        UCB1_scores.push_back(child->UCB1());
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

            static double WIN_SCORE;
            static double DRAW_SCORE;

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
    
    double Node::WIN_SCORE = 1.0;
    double Node::DRAW_SCORE = 0.0;

    inline void init(double win_score, double draw_score) {
        Node::WIN_SCORE = win_score;
        Node::DRAW_SCORE = draw_score;
    };
}