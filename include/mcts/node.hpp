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
#include <memory>

namespace node
{
    using policy_function_type = std::function<double(const chess::position&, chess::side)>;
    
    class Node : public std::enable_shared_from_this<Node>
    {
        public:
            // Used to create a node that is not a parent node
            Node(chess::position state, chess::side side, chess::side player_side, bool is_start_node, std::weak_ptr<Node> parent, chess::move move)
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
            Node(chess::position state, chess::side side, chess::side player_side) : Node(state, side, player_side, true, std::weak_ptr<Node>(), chess::move()) {}
            ~Node() = default;
            // Get child nodes
            inline std::vector<std::shared_ptr<Node>> get_children() const
            {
                return this->children;
            }

            // Perform rollout from state
            void rollout(policy_function_type rollout_policy)
            {
                t = rollout_policy(state, player_side);
                n++;
            }

            // Backpropagate score and visits to parent node
            void backpropagate()
            {
                if (auto p = parent.lock())
                {
                    p->t += t;
                    p->n++;

                    if (!p->is_start_node)
                    {
                        p->backpropagate();
                    }
                }
            }

            // Expand node
            void expand()
            {   
                std::vector<chess::move> available_moves{state.moves()};
                for (chess::move child_move : available_moves)
                {
                    chess::position child_state = state.copy_move(child_move); // TODO - Make this optional
                    std::shared_ptr<Node> new_child = std::make_shared<Node>(child_state, child_state.get_turn(), player_side, false, weak_from_this(), child_move);
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
                        new_child->is_terminal_node = true;
                        new_child->n = 1;
                        new_child->backpropagate();
                    }
                    children.push_back(new_child);
                }
            }

            // UCB1 scoring function
            inline double UCB1() const
            {
                auto p = parent.lock();
                if (n == 0 || (p && p->n==0))
                {
                    return DBL_MAX;
                }
                else
                {
                    double v_bar = t / n;
                    int N = p ? p->n : 1;
                    return v_bar + sqrt(2 * log(N) / n);
                }
            }

            // Determine next node to expand/rollout by traversing tree
            std::shared_ptr<Node> traverse()
            {
                std::vector<double> UCB1_scores{};
                for (std::shared_ptr<Node> child : children)
                {
                    if (!child->is_terminal_node)
                        UCB1_scores.push_back(child->UCB1());
                }
                if (UCB1_scores.size() == 0)
                {
                    is_terminal_node = true;
                    return parent.lock() ? parent.lock() : shared_from_this();
                }

                std::shared_ptr<Node> best_child = get_max_element<std::shared_ptr<Node>>(children.begin(), UCB1_scores.begin(), UCB1_scores.end());

                if (best_child->children.size() > 0)
                {
                    return best_child->children.front()->traverse();
                }
                else
                {
                    return best_child;
                }
            }

            // Retrieve the best child node based on UCB1 score
            // Can be useful if we want to keep the tree from the previous iterations
            std::shared_ptr<Node> best_child() const
            {
                std::vector<double> UCB1_scores{};
                for (std::shared_ptr<Node> child : children)
                {
                    UCB1_scores.push_back(child->UCB1());
                }
                return get_max_element<std::shared_ptr<Node>>(children.begin(), UCB1_scores.begin(), UCB1_scores.end());
            }
            // Get the move that gives the best child
            // Useful for baseline mcts algorithm
            chess::move best_move() const
            {
                return best_child()->move;
            }

            // Get state
            chess::position get_state() const
            {
                return state;
            }

            // Check if current state is a terminal state
            bool is_over() const
            {
                return is_terminal_node || state.is_checkmate() || state.is_stalemate();
            }

            // Get amount of vists
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
                    for(std::shared_ptr<Node> child_ptr : children)
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
            std::weak_ptr<Node> parent;
            std::vector<std::shared_ptr<Node>> children;
            double t;
            int n;
    };
    
    double Node::WIN_SCORE = 1.0;
    double Node::DRAW_SCORE = 0.0;

    // Initialize node library 
    // Sets reward scores
    // Not necessary unless modifying scores is desired
    void init(double win_score, double draw_score) {
        Node::WIN_SCORE = win_score;
        Node::DRAW_SCORE = draw_score;
    };
}