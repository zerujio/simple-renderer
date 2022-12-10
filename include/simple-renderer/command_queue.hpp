#ifndef PROCEDURALPLACEMENTLIB_COMMAND_QUEUE_HPP
#define PROCEDURALPLACEMENTLIB_COMMAND_QUEUE_HPP

#include "draw_command.hpp"

#include "glutils/program.hpp"
#include "glutils/vertex_array.hpp"

#include <vector>
#include <utility>
#include <tuple>

namespace simple {

/// A container for drawing commands.
template<typename... CommandTypes>
class CommandQueue final
{
public:
    template<typename T, std::size_t Index = 0>
    constexpr std::vector<T>& getCommandVector()
    {
        using VectorTuple = decltype(m_command_vectors);

        static_assert(Index < std::tuple_size_v<VectorTuple>, "Command type not supported by CommandQueue instance");

        if constexpr (std::is_same_v<std::tuple_element_t<Index, VectorTuple>, std::vector<T>>)
            return std::get<Index>(m_command_vectors);
        else
            return getCommandVector<T, Index + 1>();
    }

    template<typename Command, typename ... Args>
    Command& enqueue(glutils::Program program, glutils::VertexArray vertex_array, std::size_t uniform_data_index,
                     Args&&... args)
    {
        return getCommandVector<Command>().emplace_back(std::forward<Args>(args)...);
    }

    void clear()
    {
        m_clearCommandVectors();
    }

private:
    std::tuple<std::vector<std::tuple<glutils::Program, glutils::VertexArray, std::size_t, CommandTypes>>...> m_command_vectors;

    template<std::size_t N = 0>
    void m_clearCommandVectors()
    {
        if constexpr (N < std::tuple_size_v<decltype(m_command_vectors)>)
        {
            std::get<N>(m_command_vectors).clear();
            m_clearCommandVectors<N+1>();
        }
    }
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_COMMAND_QUEUE_HPP
