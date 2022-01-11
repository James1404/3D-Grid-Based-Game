#pragma once
#include <xhash>

struct uuid {
	uuid();
	uuid(uint64_t uuid);
	uuid(const uuid&) = default;

	operator uint64_t() const { return m_uuid; }
private:
	uint64_t m_uuid;
};

namespace std {
	template<>
	struct hash<uuid> {
		std::size_t operator()(const uuid& _uuid) const {
			return hash<uint64_t>()((uint64_t)_uuid);
		}
	};
}