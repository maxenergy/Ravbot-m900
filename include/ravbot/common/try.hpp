// Copyright 2025 RavBot Contributors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <type_traits>
#include <utility>

// ─── RAVBOT_TRY / RAVBOT_TRY_ASSIGN ────────────────────────────────────────────────
//
// Propagates errors up the call stack, similar to Rust's `?` operator.
//
// Given an expression that returns a StatusOr<T> (or any type that is
// contextually convertible to bool for "is OK" and supports operator* for
// value extraction), RAVBOT_TRY either:
//   • returns the error from the enclosing function if the result is falsy, or
//   • yields the unwrapped success value if truthy.
//
// Two variants:
//
//   RAVBOT_TRY_ASSIGN(var, expr)   — Cross-platform (GCC/Clang/MSVC).
//                                 Declares `var` with the unwrapped value.
//     RAVBOT_TRY_ASSIGN(val, ComputeSomething(x));   // val is now an int
//     RAVBOT_TRY_ASSIGN(data, LoadFile(path));        // data is now Bytes
//
//   RAVBOT_TRY(expr)               — GCC/Clang only (statement expression).
//                                 Can be used inline as an expression.
//     auto val = RAVBOT_TRY(ComputeSomething(x));

namespace ravbot::ravbot_detail {

template <typename T>
struct unwrap_result {
  // Return by value (not decltype(auto)) to prevent dangling references
  // when used in GCC statement expressions where the source is destroyed.
  static auto
  get(T&& v) -> std::remove_reference_t<decltype(*std::forward<T>(v))> {
    return *std::forward<T>(v);
  }
};

}  // namespace ravbot::ravbot_detail

// ── Cross-platform: RAVBOT_TRY_ASSIGN(var, expr) ────────────────────────────────
// Works on GCC, Clang, and MSVC.  Declares `var` in the enclosing scope.
#define RAVBOT_TRY_CONCAT_IMPL(a, b) a##b
#define RAVBOT_TRY_CONCAT(a, b) RAVBOT_TRY_CONCAT_IMPL(a, b)

#define RAVBOT_TRY_ASSIGN(var, ...)                                          \
  auto RAVBOT_TRY_CONCAT(_ravbot_try_r_, __LINE__) = (__VA_ARGS__);              \
  if (!static_cast<bool>(RAVBOT_TRY_CONCAT(_ravbot_try_r_, __LINE__)))           \
    return std::forward<decltype(RAVBOT_TRY_CONCAT(_ravbot_try_r_, __LINE__))>(  \
        RAVBOT_TRY_CONCAT(_ravbot_try_r_, __LINE__));                            \
  auto var = ::ravbot::ravbot_detail::                                    \
      unwrap_result<decltype(RAVBOT_TRY_CONCAT(_ravbot_try_r_, __LINE__))>::get( \
          std::forward<decltype(RAVBOT_TRY_CONCAT(_ravbot_try_r_, __LINE__))>(   \
              RAVBOT_TRY_CONCAT(_ravbot_try_r_, __LINE__)))

// ── GCC/Clang only: RAVBOT_TRY(expr) ────────────────────────────────────────────
// Usable as an expression: auto val = RAVBOT_TRY(expr);
#if defined(__GNUC__) || defined(__clang__)
#define RAVBOT_TRY(...)                                            \
  __extension__({                                              \
    auto _r_ = (__VA_ARGS__);                                  \
    if (!static_cast<bool>(_r_))                               \
      return std::forward<decltype(_r_)>(_r_);                 \
    ::ravbot::ravbot_detail::unwrap_result<decltype(_r_)>::get( \
        std::forward<decltype(_r_)>(_r_));                     \
  })
#endif
