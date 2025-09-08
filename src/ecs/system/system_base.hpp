// ReSharper disable CppCStyleCast
#pragma once
#include "entt/fwd.hpp"
#include "entt/entity/registry.hpp"

namespace ewsm
{
	// 1. Bind registry
	// 2. Delete copy/move ctor/assign
	// 3. Put self into reg's context, providing Get/TryGet function
	template <typename System>
	struct SystemBase
	{
		SystemBase(entt::registry& registry);
		virtual ~SystemBase() = default;

		SystemBase(const SystemBase&) = delete;
		SystemBase(SystemBase&&) = delete;
		SystemBase& operator=(const SystemBase&) = delete;
		SystemBase& operator=(SystemBase&&) = delete;

		static System& Get(const entt::registry& reg)
		{
			return *(reg.ctx().get<System*>());
		}

		static System* TryGet(const entt::registry& reg)
		{
			if (auto ret = reg.ctx().find<System*>()) {
				return *ret;
			}
			return nullptr;
		}

		entt::registry& reg;
	};

	template <typename System>
	SystemBase<System>::SystemBase(entt::registry& registry)
		: reg(registry)
	{
		reg.ctx().emplace<System*>((System*)this);
	}
}
