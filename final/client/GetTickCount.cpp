#include <chrono>

#include "GetTickCount.hpp"

inline int GetTickCount() {
	auto now = std::chrono::steady_clock::now();
	return now.time_since_epoch().count();
}
