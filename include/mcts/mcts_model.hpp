#ifndef MODEL_H
#define MODEL_H

#include <mcts/node.hpp>
#include <chess/chess.hpp>

#include <memory>

// Model struct to simplify usage of the search
struct Model
{
    using policy_function_type = node::policy_function_type;

    Model(policy_function_type rollout_policy, chess::position state, chess::side model_side) 
        : rollout_policy{rollout_policy},
    {
        this->main_node = std::make_shared<node::Node>(state, state.get_turn(), model_side)
    }

    policy_function rollout_policy;
    std::shared_ptr<node::Node> main_node;
}

#endif /* MODEL_H */