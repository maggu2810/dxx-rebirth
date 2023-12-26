#include "d_range.h"
#include <ranges>
#include <vector>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Rebirth xrange
#include <boost/test/unit_test.hpp>

/* Test that an xrange is empty when the ending bound is 0.
 */
BOOST_AUTO_TEST_CASE(xrange_empty_0)
{
	bool empty = true;
	for (auto &&v : xrange(0u))
	{
		(void)v;
		empty = false;
	}
	BOOST_TEST(empty);
}

/* Test that an xrange is empty when the start is higher than the end.
 */
BOOST_AUTO_TEST_CASE(xrange_empty_transposed)
{
	bool empty = true;
	for (auto &&v : xrange(2u, 1u))
	{
		(void)v;
		empty = false;
	}
	BOOST_TEST(empty);
}

/* Test that an xrange produces the correct number of entries.
 */
BOOST_AUTO_TEST_CASE(xrange_length)
{
	unsigned count = 0;
	constexpr unsigned length = 4u;
	for (auto &&v : xrange(length))
	{
		(void)v;
		++ count;
	}
	BOOST_TEST(count == length);
}

/* Test that an xrange produces the correct values when using an implied
 * start of 0.
 */
BOOST_AUTO_TEST_CASE(xrange_contents)
{
	std::vector<unsigned> out;
	for (auto &&v : xrange(4u))
		out.emplace_back(v);
	std::vector<unsigned> expected{0, 1, 2, 3};
	BOOST_TEST(out == expected);
}

/* Test that an xrange produces the correct values when using an
 * explicit start.
 */
BOOST_AUTO_TEST_CASE(xrange_contents_start)
{
	std::vector<unsigned> out;
	for (auto &&v : xrange(2u, 4u))
		out.emplace_back(v);
	std::vector<unsigned> expected{2, 3};
	BOOST_TEST(out == expected);
}

/* Test that an xrange produces the correct values when stepping up
 * with skipped values.
 */
BOOST_AUTO_TEST_CASE(xrange_contents_start_up_2)
{
	std::vector<unsigned> out;
	for (auto &&v : xrange(std::integral_constant<unsigned, 2u>(), std::integral_constant<unsigned, 6u>(), std::integral_constant<int, 2>()))
		out.emplace_back(v);
	std::vector<unsigned> expected{2, 4};
	BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(xrange_contents_descending)
{
	std::vector<unsigned> out;
	for (auto &&v : xrange(std::integral_constant<unsigned, 4u>(), std::integral_constant<unsigned, 2u>(), xrange_descending()))
		out.emplace_back(v);
	std::vector<unsigned> expected{4, 3};
	BOOST_TEST(out == expected);
}

/* Test that an xrange produces the correct values when stepping down
 * with skipped values.
 */
BOOST_AUTO_TEST_CASE(xrange_contents_start_down_2_constant)
{
	std::vector<unsigned> out;
	for (auto &&v : xrange(std::integral_constant<unsigned, 5u>(), std::integral_constant<unsigned, 1u>(), std::integral_constant<int, -2>()))
		out.emplace_back(v);
	std::vector<unsigned> expected{5, 3};
	BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(xrange_contents_start_down_variable)
{
	std::vector<unsigned> out;
	for (auto &&v : xrange(5u, std::integral_constant<unsigned, 2u>(), xrange_descending()))
		out.emplace_back(v);
	std::vector<unsigned> expected{5, 4, 3};
	BOOST_TEST(out == expected);
}

/* Test that pairs of iterators pulled from a shared xrange are
 * independent.  If traversing an xrange modified the state of the
 * xrange object, this test would either hang due to the inner loop
 * resetting state, or produce incorrect output.
 */
BOOST_AUTO_TEST_CASE(xrange_self_nest)
{
	std::vector<unsigned> out;
	auto &&r = xrange(2u);
	for (auto &&ov : r)
		for (auto &&iv : r)
			out.emplace_back((ov << 8) | iv);
	std::vector<unsigned> expected{0, 1, (1 << 8), (1 << 8) + 1};
	BOOST_TEST(out == expected);
}

BOOST_AUTO_TEST_CASE(xrange_iter_values)
{
	auto &&r = xrange(2u);
	BOOST_TEST(*r.begin() == 0);
	BOOST_TEST(*std::next(r.begin()) == 1);
	BOOST_TEST(*std::next(r.begin(), 2) == 2);
}

static_assert(std::ranges::borrowed_range<decltype(xrange(2u))>);
