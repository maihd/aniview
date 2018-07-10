#ifndef __ARRAY_SEARCH_HPP__
#define __ARRAY_SEARCH_HPP__

#include "array.hpp"
#include "array.search.hpp"

namespace array
{
    template <typename type_t>
    inline int bsearch(const array_t<type_t>& array, const type_t& value)
    {
        return search::binary(array, array.count, value);
    }

    template <typename type_t, typename func_t>
    inline int bsearch(const array_t<type_t>& array, const type_t& value, const func_t& compare)
    {
        return search::binary(array, array.count, value, compare);
    }
}

#endif /* __ARRAY_SEARCH_HPP__ */