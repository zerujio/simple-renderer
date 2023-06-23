#ifndef PROCEDURALPLACEMENTLIB_COMMAND_COLLECTOR_HPP
#define PROCEDURALPLACEMENTLIB_COMMAND_COLLECTOR_HPP

#include "command_queue.hpp"
#include "glutils/program.hpp"
#include "glutils/vertex_array.hpp"

namespace Simple {

/**
 * @brief A structure that keeps a reference to a CommandQueue and allows new commands to be enqueued.
 * @tparam CommandQueueType A CommandQueue instantiation.
 */
template<typename ... CommandTypes>
class CommandCollector
{
public:
    CommandCollector(CommandQueue<CommandTypes...> &command_queue, std::size_t uniform_data_index, GL::ProgramHandle program) :
            m_command_queue(command_queue),
            m_bound_args(uniform_data_index, program)
    {}

    template<typename CommandType>
    void emplace(CommandType &&command, GL::VertexArrayHandle vertex_array) const
    {
        m_command_queue.template emplace<std::decay_t<CommandType>>(std::forward<CommandType>(command),
                std::tuple_cat(m_bound_args, std::make_tuple(vertex_array)));
    }

private:
    CommandQueue<CommandTypes...>& m_command_queue;
    std::tuple<std::size_t, GL::ProgramHandle> m_bound_args;
};

} // simple

#endif //PROCEDURALPLACEMENTLIB_COMMAND_COLLECTOR_HPP
