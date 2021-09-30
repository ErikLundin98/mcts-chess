#ifndef MODEL_H
#define MODEL_H

#include <mcts/node.hpp>
#include <chess/chess.hpp>

#include <memory>

namespace mcts_model
{
    using policy_function_type = node::policy_function_type;

    // Model struct to simplify usage of the search
    struct Model
    {
        Model(policy_function_type rollout_policy, chess::side model_side) 
            : rollout_policy{rollout_policy},
            model_side{model_side}
        {}

        chess::move search(chess::position state, int max_iter)
        {
            std::shared_ptr<node::Node> main_node{std::make_shared<node::Node>(state, model_side)};
            main_node->expand();
            for(int i = 0 ; i < max_iter ; ++i)
            {
                std::shared_ptr<node::Node> current_node = main_node->traverse();
                if(current_node->is_over()) break;
                if(current_node->get_n() != 0)
                {
                    current_node->expand();
                    current_node = current_node->get_children().front();
                }
                current_node->rollout(rollout_policy);
                current_node->backpropagate();
            }
            return main_node->best_move();
        }

        policy_function_type rollout_policy;
        chess::side model_side;
    };

}



#endif /* MODEL_H */