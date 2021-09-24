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

template<typename Iterator>
size_t get_max_idx(Iterator start, Iterator end)
{
    return std::distance(start, std::max_element(start, end));
}

template<typename T, typename Iterator1, typename Iterator2>
T get_max_element(Iterator1 wanted_start, Iterator2 comparator_start, Iterator2 comparator_end)
{
    size_t max_idx = get_max_idx(comparator_start, comparator_end);
    std::advance(wanted_start, max_idx);
    return *wanted_start;
}