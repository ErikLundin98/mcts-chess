#include <random>
#include <iterator>

template<typename Iterator, typename RandomGenerator>
inline Iterator random_element(Iterator start, Iterator end, RandomGenerator& generator)
{
    std::uniform_int_distribution<> dist(0, std::distance(start, end)-1);
    std::advance(start, dist(generator));
    return start;
}

template<typename Iterator>
inline Iterator random_element(Iterator start, Iterator end)
{
    static std::random_device random_device;
    static std::mt19937 generator(random_device());
    return random_element(start, end, generator);
}

