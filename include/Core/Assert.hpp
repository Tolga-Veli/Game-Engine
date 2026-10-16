#pragma once

#include "Logging.hpp"

#include <cstdlib>
#include <source_location>
#include <string_view>

#if defined(_MSC_VER)
#define CORE_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define CORE_DEBUG_BREAK() __builtin_trap()
#else
#include <csignal>
#define CORE_DEBUG_BREAK() raise(SIGTRAP)
#endif

namespace Jupiter::Core::detail {

// This is a hard-failure path, and if a future caller wants to
// turn it into a throw instead of abort() (e.g. for a fuzzing harness), that
// shouldn't require touching the macros.
[[noreturn]] inline void AssertFail(std::string_view expr, std::string_view msg, std::source_location loc) {
  Logger::Instance().Log(LogLevel::Fatal, loc, "Assertion failed: ({}) {}", expr, msg);
  CORE_DEBUG_BREAK();
  std::abort();
}

[[noreturn]] inline void UnreachableFail(std::string_view msg, std::source_location loc) {
  Logger::Instance().Log(LogLevel::Fatal, loc, "Unreachable code reached: {}", msg);
  CORE_DEBUG_BREAK();
  std::abort();
}

} // namespace Jupiter::Core::detail

// Always active, in debug AND release. Use for conditions that must never
// be violated without immediately corrupting program state. If you're ever
// tempted to disable this in release "for performance", that's a sign the
// check belongs in CORE_ASSERT instead, not that VERIFY should be weaker.
#define CORE_VERIFY(cond, msg)                                                                                         \
  do {                                                                                                                 \
    if (!(cond)) {                                                                                                     \
      ::Jupiter::Core::detail::AssertFail(#cond, msg, std::source_location::current());                                \
    }                                                                                                                  \
  } while (false)

// Compiled out entirely in release - the condition itself is never
// evaluated, so this costs nothing and won't trigger unused-variable
// warnings on release builds. Use for expensive or redundant sanity checks
// you only want while developing (bounds checks, invariant re-verification,
// canary checks on every allocation, etc).
//
// Define CORE_FORCE_ASSERTS to keep these live in a release build too
#if !defined(NDEBUG) || defined(CORE_FORCE_ASSERTS)
#define CORE_ASSERT(cond, msg) CORE_VERIFY(cond, msg)
#define CORE_ENABLE_ASSERTS 1
#else
#define CORE_ASSERT(cond, msg) ((void)0)
#define CORE_ENABLE_ASSERTS 0
#endif

#define UNREACHABLE(msg)                                                                                               \
  do {                                                                                                                 \
    ::Jupiter::Core::detail::UnreachableFail(msg, std::source_location::current());                                    \
  } while (false)
