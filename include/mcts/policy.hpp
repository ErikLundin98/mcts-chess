#pragma once

#include <functional>
#include <mcts/node.hpp>
#include <chess/chess.hpp>
#include <iostream>

inline double random_rollout(chess::position state, chess::side player_turn, std::mt19937 & generator)
{
    double accumulated_t{0};
    int n_rollouts{10};
    for (int i = 0; i < n_rollouts; ++i)
    {
        chess::position rollout_state = state;
        // by default, do random rollouts
        short uneventful_timer = 0;
        while (!rollout_state.is_checkmate() && !rollout_state.is_stalemate() && uneventful_timer < 50)
        {
            std::vector<chess::move> available_moves{rollout_state.moves()};
            chess::move random_choice = *random_element(std::begin(available_moves), std::end(available_moves), generator);
            chess::undo undo = rollout_state.make_move(random_choice);
            uneventful_timer = undo.capture != chess::piece::piece_none ? 0 : uneventful_timer + 1;
        }

        bool is_player_turn = rollout_state.get_turn() == player_turn;
        if (rollout_state.is_checkmate())
        {
            accumulated_t += is_player_turn ? -1 : 1; // TODO: Parametrize
        }
        else
        {
            accumulated_t += 0; //TODO
        }
    }
    
    return accumulated_t / n_rollouts;
};

inline double bad_rollout(chess::position state, chess::side player_turn)
{
    return 0;
}