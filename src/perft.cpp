#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <chrono>

#include <chess/chess.hpp>

using namespace chess;

struct entry
{
    std::size_t hash;
    unsigned long long nodes;
    int depth;
};

struct result
{
    std::string fen;
    std::vector<unsigned long long> nodes;
};

static std::unordered_map<std::size_t, entry> table;
static const size_t table_key_size = 24;
static const size_t table_hash_mask = (1ULL << table_key_size) - 1;
unsigned int        table_hits{0};

static std::unordered_map<std::string, result> results
{
    {"startpos", {std::string(position::fen_start), {1, 20, 400, 8902, 197281, 4865609, 119060324, 3195901860, 84998978956, 2439530234167, 69352859712417}}},
    {"p2", {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", {1, 48, 2039, 97862, 4085603, 193690690, 8031647685}}},
    {"p3", {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", {1, 14, 191, 2812, 43238, 674624, 11030083, 178633661, 3009794393}}},
    {"p4", {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", {1, 6, 264, 9467, 422333, 15833292, 706045033}}},
    {"p5", {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", {1, 44, 14686, 62379, 2103487, 89941194}}},
    {"p6", {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", {1, 46, 2079, 89890, 3894594, 164075551, 6923051137, 287188994746, 11923589843526, 490154852788714}}},
};


unsigned long long perft(int depth, position& p, bool divide = false)
{
    if(depth == 0) return 1;

    std::size_t hash = p.hash();
    std::size_t key = hash & table_hash_mask;
    
    if(table.find(key) != table.end() && table.at(key).hash == hash && table.at(key).depth == depth)
    {
        table_hits++;
        return table.at(key).nodes;
    }

    unsigned long long nodes = 0;

    for(move& move: p.moves())
    {
        undo undo = p.make_move(move);
        unsigned long long move_nodes = perft(depth - 1, p);
        p.undo_move(move, undo);

        nodes += move_nodes;
        
        if(divide)
        {
            std::cout << move.to_lan() << ": " << move_nodes << std::endl;
        }
    }

    table.emplace(key, entry{hash, nodes, depth});

    return nodes;
}

int test(int depth = 5)
{
    for(auto& [test, result]: results)
    {
        std::cerr << "perft test '" << test << "': ";

        position p = position::from_fen(result.fen);    
        unsigned long long nodes = perft(depth, p, true);
        unsigned long long answer = result.nodes[depth];

        if(nodes == answer)
        {
            std::cerr << "success" << std::endl;
        }
        else
        {
            std::cerr << "failure" << std::endl;
            return 1;
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    chess::init();

    // run tests if no argument is supplied
    if(argc == 1)
    {
        return test();
    }

    // otherwise run input position
    std::string fen = argv[1];
    int depth = std::stoi(argv[2]);

    result answer = results.find(fen) != results.end() ? results.at(fen) : result{fen, {}};
    position p = position::from_fen(answer.fen);
    
    auto begin = std::chrono::steady_clock::now();
    unsigned long long nodes = perft(depth, p, true);
    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> time = end - begin;

    double nps = (1e9*nodes) / time.count();

    std::cout << "time: " << time.count() << " s (" << nps << " nps, " << table_hits << " hits)" << std::endl;
    std::cout << "total: " << nodes << std::endl;

    if(static_cast<unsigned>(depth) < answer.nodes.size())
    {
        std::cout << "expected: " << answer.nodes[depth] << std::endl;
        
        if(nodes != answer.nodes[depth])
        {
            return 1;
        }
    }

    return 0;
}
