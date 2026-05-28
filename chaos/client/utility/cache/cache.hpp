#pragma once

#include "client/sdk/sdk.hpp"
#include <vector>
#include <mutex>
#include <memory>
#include <atomic>

template<typename T>
class atomic_ptr {
public:
	atomic_ptr(T* value = nullptr) noexcept : ptr(value) {}

	auto store(T* value, std::memory_order order = std::memory_order_seq_cst) noexcept -> void {
		ptr.store(value, order);
	}

	auto load(std::memory_order order = std::memory_order_seq_cst) const noexcept -> T* {
		return ptr.load(order);
	}

	auto operator->() const noexcept -> T* {
		return load(std::memory_order_acquire);
	}

	explicit operator T*() const noexcept {
		return load(std::memory_order_acquire);
	}

private:
	std::atomic<T*> ptr;
};

struct actor_t {
	sdk::a_pawn m_pawn;
	sdk::a_character m_mesh;
	sdk::a_player_state m_player_state;
	int m_team_index;

	math::f_vector m_head_pos;
	math::f_vector m_foot_pos;
	std::vector<std::pair<math::f_vector, math::f_vector>> m_skeleton_lines;
	bool m_bones_valid = false;
};

class c_cache {
public:
	std::atomic<sdk::u_world*> uworld = nullptr;
	std::atomic<sdk::u_game_instance*> game_instance = nullptr;
	std::atomic<sdk::u_local_player*> local_player = nullptr;
	std::atomic<sdk::a_player_controller*> player_controller = nullptr;
	atomic_ptr<sdk::a_character> acknowledged_pawn = nullptr;
	std::atomic<sdk::a_game_state_base*> game_state = nullptr;
	std::atomic<uintptr_t> player_array = 0;
	std::atomic<int> player_array_size = 0;

	std::vector<actor_t> actor_list;
	std::vector<actor_t> temporary_actor_list;
	std::mutex actor_mutex;

	auto read_bone( uintptr_t mesh , int bone_id ) -> math::f_vector;

	auto data() -> void;
	auto entities() -> void;
};

inline auto cache = std::make_shared<c_cache>();
