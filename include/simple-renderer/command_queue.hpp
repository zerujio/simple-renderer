#ifndef PROCEDURALPLACEMENTLIB_COMMAND_QUEUE_HPP
#define PROCEDURALPLACEMENTLIB_COMMAND_QUEUE_HPP

#include "type_set.hpp"

#include "glutils/vertex_array.hpp"
#include "glutils/program.hpp"

#include <vector>
#include <utility>
#include <tuple>

namespace simple {

namespace impl {
struct ClearVector {
    template<typename T>
    void operator() (std::vector<T>& v) const { v.clear(); }
};
}

/**
 * @brief A container for drawing commands.
 * @tparam CommandSetType An instantiation of TypeSet
 * @tparam CommandArgsType A tuple-like holding additional arguments common to all command types.
 */
template<typename ... CommandTypes>
class CommandQueue final
{
public:
    using CommandSet = TypeSet<CommandTypes...>;

    using CommandArgs = std::tuple<std::size_t, glutils::Program, glutils::VertexArray>;

    template<typename Command>
    using CommandVector = std::vector<std::pair<Command, CommandArgs>>;

    template<typename CommandType>
    static constexpr std::size_t command_type_index = CommandSet::template type_index<CommandType>;

    /**
     * @brief Access all commands of a specific type.
     * @tparam Command The type of command to access.
     * @return A vector of pairs, each one holding a command and its arguments.
     */
    template<typename Command>
    [[nodiscard]]
    const CommandVector<Command>& getCommands() const
    {
        return std::get<command_type_index<Command>>(m_command_vectors);
    }

    template<typename Command>
    [[nodiscard]]
    CommandVector<Command>& getCommands()
    {
        return std::get<command_type_index<Command>>(m_command_vectors);
    }

    /**
     * @brief Insert a new command-args pair into the queue.
     * @tparam PairArgs argument types for the std::pair constructor.
     * @tparam args Arguments to forward to the std::pair<CommandType, CommandArgs> constructor.
     * @return a reference to the constructed pair.
     */
    template<typename CommandType, typename ... PairArgs>
    std::pair<CommandType, CommandArgs>& emplace(PairArgs&&... args)
    {
        return getCommands<CommandType>().emplace_back(std::forward<PairArgs>(args)...);
    }

    /// clear all commands of all types.
    void clear() { forEachCommandType<impl::ClearVector>(); }

    /// Invokes a functor once for each command type. The functor must have a templated function call operator.
    template<typename Func>
    void forEachCommandType(const Func& func = Func()) { m_forEachCommandType(func); }

private:
    std::tuple<CommandVector<CommandTypes>...> m_command_vectors;

    template<typename Func, std::size_t I = 0>
    void m_forEachCommandType(const Func& func)
    {
        if constexpr (I < sizeof...(CommandTypes))
        {
            func(std::get<I>(m_command_vectors));
            m_forEachCommandType<Func, I + 1>(func);
        }
    }
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_COMMAND_QUEUE_HPP
