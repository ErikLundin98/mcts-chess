#pragma once

#include <chess/chess.hpp>

class Node
{
    public:
        Node(chess::position state, Node* parent=nullptr) : state{state}, parent{parent}
        {}

    private:
        chess::position state;
        Node* parent;
};