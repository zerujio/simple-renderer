#include "ostream_operators.hpp"
#include "catch.hpp"

#include "simple-renderer/vertex_buffer.hpp"

#include "glm/glm.hpp"

namespace Catch::Generators {

template<uint L, typename T>
class VectorGenerator : public IGenerator<glm::vec<L, T>>
{
public:
    static_assert(1 <= L && L <= 4);

    explicit VectorGenerator(GeneratorWrapper<T>&& gen) : m_generator(std::move(gen))
    {
        m_next_value[0] = m_generator.get();
        if (!nextI<1>())
            throw_exception(GeneratorException("Not enough values to initialize VectorGenerator"));
    }

    bool next() override
    {
        return nextI<0>();
    }

    const glm::vec<L, T> &get() const override
    {
        return m_next_value;
    }

private:
    template<uint I>
    bool nextI()
    {
        if constexpr (I < L)
        {
            if (!m_generator.next())
                return false;

            m_next_value[I] = m_generator.get();
            return nextI<I + 1>();
        }
        else
            return true;
    }

    glm::vec<L, T> m_next_value;
    GeneratorWrapper<T> m_generator;
};

template<uint L, typename T>
GeneratorWrapper<glm::vec<L, T>> vec(GeneratorWrapper<T>&& generator)
{
    return GeneratorWrapper<glm::vec<L, T>>(std::make_unique<VectorGenerator<L, T>>(std::move(generator)));
}

#define DEFINE_VEC_GENERATORS(PREFIX, TYPE) \
auto PREFIX##vec1(GeneratorWrapper<TYPE>&& generator) { return vec<1, TYPE>(std::move(generator)); } \
auto PREFIX##vec2(GeneratorWrapper<TYPE>&& generator) { return vec<2, TYPE>(std::move(generator)); } \
auto PREFIX##vec3(GeneratorWrapper<TYPE>&& generator) { return vec<3, TYPE>(std::move(generator)); } \
auto PREFIX##vec4(GeneratorWrapper<TYPE>&& generator) { return vec<4, TYPE>(std::move(generator)); }

DEFINE_VEC_GENERATORS(, float)
DEFINE_VEC_GENERATORS(d, double)
DEFINE_VEC_GENERATORS(i, int)
DEFINE_VEC_GENERATORS(u, uint)
DEFINE_VEC_GENERATORS(b, bool)

} // namespace Catch::Generators

template<std::size_t I, typename ... Ts>
auto readSection(const Simple::Renderer::VertexBuffer<Ts...>& vertex_buffer)
{
    auto range = vertex_buffer.template getTypedRange<I>();
    using Type = typename Simple::Renderer::VertexBuffer<Ts...>::template VertexTypeByIndex<I>;

    std::vector<Type> values(range.size);

    vertex_buffer.getBuffer().getGLHandle().read(range.offset.get(), range.size * sizeof(Type), values.data());

    return values;
}

TEST_CASE("Single-type VertexBuffer")
{
    auto values = GENERATE(take(1, chunk(100, vec3(random(std::numeric_limits<float>::lowest(),
                                                          std::numeric_limits<float>::max())))));

    Simple::Renderer::VertexBuffer<glm::vec3> vertex_buffer {values};

    CHECK(values == readSection<0>(vertex_buffer));
}

TEST_CASE("Multi-type VertexBuffer")
{
    constexpr float float_max = std::numeric_limits<float>::max();
    constexpr float float_min = std::numeric_limits<float>::lowest();

    auto a_values = GENERATE(take(2, chunk(100, vec2(random(float_min, float_max)))));
    auto b_values = GENERATE(take(2, chunk(75, vec3(random(float_min, float_max)))));
    auto c_values = GENERATE(take(2, chunk(50, ivec4(random(std::numeric_limits<int>::lowest(),
                                                            std::numeric_limits<int>::max())))));

    Simple::Renderer::VertexBuffer<glm::vec2, glm::vec3, glm::ivec4> vertex_buffer
    {
        a_values, b_values, c_values
    };

    CHECK(a_values == readSection<0>(vertex_buffer));
    CHECK(b_values == readSection<1>(vertex_buffer));
    CHECK(c_values == readSection<2>(vertex_buffer));
}