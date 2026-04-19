#pragma once

#include <fmt/format.h>

#define LOG_MESSAGE(...) fmt::println(__VA_ARGS__)
#define LOG_CRITICAL(...) throw std::runtime_error(fmt::format(__VA_ARGS__))
