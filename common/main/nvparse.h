/*
 * This file is copyright by Rebirth contributors and licensed as
 * described in COPYING.txt.
 * See COPYING.txt for license details.
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <optional>
#include <ranges>
#include <type_traits>
#include "dxxsconf.h"
#include "ntstring.h"

template <std::size_t N>
requires(N > 1)
[[nodiscard]]
static inline bool compare_nonterminated_name(const auto &candidate, const char (&name)[N])
{
	/* C++ rules for string literals guarantee that `name` has a trailing null
	 * terminator.  Callers pass `candidate` with no trailing null terminator,
	 * so the simple invocation of `std::ranges::equal(range, range)` will
	 * always fail to match.
	 *
	 * `compare_nonterminated_name` considers the candidate equal to the name
	 * if the name without the null terminator compares equal to the candidate,
	 * so use `std::prev` to exclude the terminator.
	 */
	return std::ranges::equal(std::ranges::begin(candidate), std::ranges::end(candidate), std::ranges::begin(name), std::prev(std::ranges::end(name)));
}

template <std::integral T>
[[nodiscard]]
static inline std::optional<T> convert_integer(const char *const value, int base, auto libc_str_to_integer)
{
	char *e;
	const auto r = libc_str_to_integer(value, &e, base);
	if (*e)
		/* Trailing garbage found */
		return std::nullopt;
	if constexpr (std::is_same<T, bool>::value)
	{
		if (r != 0 && r != 1)
			return std::nullopt;
	}
	else if (!std::in_range<T>(r))
		/* Result truncated */
		return std::nullopt;
	return static_cast<T>(r);
}

template <std::integral T>
[[nodiscard]]
static inline auto convert_integer(const std::span<char>::iterator value, int base, auto libc_str_to_integer)
{
	return convert_integer<T>(value.base(), base, libc_str_to_integer);
}

template <std::signed_integral T>
[[nodiscard]]
static inline auto convert_integer(auto &&value, int base = 10)
{
	return convert_integer<T>(std::move(value), base, std::strtol);
}

template <std::unsigned_integral T>
[[nodiscard]]
static inline auto convert_integer(auto &&value, int base = 10)
{
	return convert_integer<T>(std::move(value), base, std::strtoul);
}

template <std::integral T>
static inline void convert_integer(T &t, auto &&value, int base = 10)
{
	if (const auto r{convert_integer<T>(std::move(value), base)})
		t = *r;
}

template <std::size_t N, typename I>
static inline void convert_string(ntstring<N> &out, I value, I eol)
{
	assert(*eol == 0);
	std::advance(eol, 1);
	const std::size_t i = std::distance(value, eol);
	if (i > out.size())
		/* Only if not truncated */
		return;
	std::copy(std::move(value), std::move(eol), out.begin());
}
