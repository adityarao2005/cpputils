#pragma once

#include <cpputils/cpputils_api.h>
#include <vector>
#include <list>
#include <stack>
#include <queue>
#include <deque>
#include <array>
#include <forward_list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <tuple>

namespace cpputils
{
    // Load all the important collections
    template <class T>
    using array_list = std::vector<T>;

    template <class T>
    using linked_list = std::list<T>;

    template <class T>
    using stack = std::stack<T>;

    template <class T>
    using queue = std::queue<T>;

    template <class T>
    using deque = std::deque<T>;

    template <class T>
    using singly_linked_list = std::forward_list<T>;

    template <class T>
    using tree_set = std::set<T>;

    template <class T>
    using hash_set = std::unordered_set<T>;

    template <class K, class V>
    using tree_map = std::map<K, V>;

    template <class K, class V>
    using hash_map = std::unordered_map<K, V>;

    template <class... T>
    using tuple = std::tuple<T...>;

    template <class T, size_t L>
    using array = std::array<T, L>;
}