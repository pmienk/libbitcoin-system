/**
 * Copyright (c) 2011-2023 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_SYSTEM_COLLECTION_IPP
#define LIBBITCOIN_SYSTEM_COLLECTION_IPP

#include <algorithm>
#include <iterator>
#include <cstddef>
#include <iterator>
#include <iostream>
#include <type_traits>
#include <vector>
#include <bitcoin/system/utility/data.hpp>

namespace libbitcoin {

template <typename Source, typename Target>
std::vector<Target> cast(const std::vector<Source>& source)
{
    std::vector<Target> target(source.size());
    target.assign(source.begin(), source.end());
    return target;
}

template <typename Container, typename Element>
bool contains(const Container& list, const Element& value)
{
    return std::any_of(list.begin(), list.end(),
        [&value](const Element& element)
        {
            return element == value;
        });
}

template <typename Element>
std::vector<Element>& distinct(std::vector<Element>& list)
{
    std::sort(list.begin(), list.end());
    list.erase(std::unique(list.begin(), list.end()), list.end());
    list.shrink_to_fit();
    return list;
}

template <typename Pair, typename Key>
int find_pair_position(const std::vector<Pair>& list, const Key& key)
{
    const auto position = std::find_if(list.begin(), list.end(),
        [&key](const Pair& pair)
        {
            return pair.first == key;
        });

    // TODO: guard cast.
    return position == list.end() ? -1 : 
        static_cast<int>(std::distance(list.begin(), position));
}

template <typename Element, typename Container>
int find_position(const Container& list, const Element& value)
{
    const auto position = std::find(std::begin(list), std::end(list), value);

    // TODO: guard cast.
    return position == std::end(list) ? -1 :
        static_cast<int>(std::distance(std::begin(list), position));
}

template <typename Type, typename Predicate>
typename std::vector<Type>::iterator insert_sorted(std::vector<Type>& list,
    Type& element, Predicate predicate)
{
    return list.insert(std::upper_bound(list.begin(), list.end(), element,
        predicate), element);
}

template <typename Type>
void move_append(std::vector<Type>& target, std::vector<Type>& source)
{
    target.reserve(target.size() + source.size());
    std::move(source.begin(), source.end(), std::back_inserter(target));
    source.clear();
}

template <typename Element>
Element pop(std::vector<Element>& stack)
{
    BITCOIN_ASSERT(!stack.empty());
    const auto element = stack.back();
    stack.pop_back();
    ////stack.shrink_to_fit();
    return element;
}

template <typename Collection>
Collection reverse(const Collection& source)
{
    Collection out(source.size());
    std::reverse_copy(std::begin(source), std::end(source), std::begin(out));
    return out;
}

} // namespace libbitcoin

// TODO: clean up std namespace.
namespace std {

template <class Type>
std::ostream& operator<<(std::ostream& output, const std::vector<Type>& list)
{
    size_t current = 0;
    const auto end = list.size();

    for (const auto& element: list)
    {
        output << element;

        if (++current < end)
            output << std::endl;
    }

    return output;
}

} // namespace std

#endif
