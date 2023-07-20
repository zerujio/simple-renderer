#ifndef PROCEDURALPLACEMENTLIB_TYPE_SET_HPP
#define PROCEDURALPLACEMENTLIB_TYPE_SET_HPP

#include <type_traits>

namespace Simple {

template<typename ... Types> struct TypeSet;

template<>
struct TypeSet<> final
{
    template<typename T>
    static constexpr std::size_t getTypeIndex()
    {
        static_assert(std::is_void_v<T>, "specified type is not part of the TypeSet");
        return 0;
    };

    template<typename T>
    static constexpr std::size_t type_index = getTypeIndex<T>();

    static constexpr std::size_t type_count = 0;
};

template<typename Head, typename ... Tail>
struct TypeSet<Head, Tail...> final
{
    template<typename T>
    static constexpr std::size_t getTypeIndex()
    {
        if constexpr (std::is_same_v<T, Head>)
            return 0;
        else
            return 1 + TypeSet<Tail...>::template type_index<T>;
    }

    template<typename T>
    static constexpr std::size_t type_index = getTypeIndex<T>();

    static constexpr std::size_t type_count = 1 + sizeof...(Tail);

    /// Instantiate a template using the types in the set as arguments.
    template<template<typename...> class Class>
    using Instantiate = Class<Head, Tail...>;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_TYPE_SET_HPP
