#pragma once

namespace ur
{

using UpdaterID = size_t;

namespace Internal
{
inline size_t GetUniqueUpdaterID() noexcept
{
	static UpdaterID id{ 0u };
	return id++;
}
}

template <typename T>
inline UpdaterID GetUpdaterTypeID() noexcept
{
	static_assert(std::is_base_of<UniformUpdater, T>::value,
		"T must inherit from UniformUpdater");

	static UpdaterID type_id{ Internal::GetUniqueUpdaterID() };
	return type_id;
}

#ifdef DrawState
#undef DrawState
#endif

class Context;
struct DrawState;

class UniformUpdater
{
public:
    virtual ~UniformUpdater() {}

    virtual UpdaterID UpdaterTypeID() const = 0;

    virtual void Update(const Context& ctx,
        const DrawState& draw, const void* scene = nullptr) = 0;

}; // UniformUpdater

}