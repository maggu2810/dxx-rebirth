/*
 * Portions of this file are copyright Rebirth contributors and licensed as
 * described in COPYING.txt.
 * Portions of this file are copyright Parallax Software and licensed
 * according to the Parallax license below.
 * See COPYING.txt for license details.

THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

/*
 *
 * string manipulation utility code
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "u_mem.h"
#include "dxxerror.h"
#include "strutil.h"
#include "inferno.h"

#include "compiler-range_for.h"
#include "d_zip.h"

namespace dcx {

#ifdef macintosh
void snprintf(char *out_string, int size, char * format, ... )
{
	va_list		args;
	char		buf[1024];
	
	va_start(args, format );
	vsprintf(buf,format,args);
	va_end(args);

	// Hack! Don't know any other [simple] way to do this, but I doubt it would ever exceed 1024 long.
	Assert(strlen(buf) < 1024);
	Assert(size < 1024);

	strncpy(out_string, buf, size);
}
#endif // macintosh

// string compare without regard to case

#ifndef DXX_HAVE_STRCASECMP
int d_stricmp( const char *s1, const char *s2 )
{
	for (;; ++s1, ++s2)
	{
		auto u1 = toupper(static_cast<unsigned>(*s1));
		auto u2 = toupper(static_cast<unsigned>(*s2));
		if (u1 != u2)
			return (u1 > u2) ? 1 : -1;
		if (!u1)
			return u1;
	}
}

int d_strnicmp(const char *s1, const char *s2, uint_fast32_t n)
{
	for (; n; ++s1, ++s2, --n)
	{
		auto u1 = toupper(static_cast<unsigned>(*s1));
		auto u2 = toupper(static_cast<unsigned>(*s2));
		if (u1 != u2)
			return (u1 > u2) ? 1 : -1;
		if (!u1)
			return u1;
	}
	return 0;
}
#endif

void d_strlwr( char *s1 )
{
	while( *s1 )	{
		*s1 = tolower(*s1);
		s1++;
	}
}

#if DXX_USE_EDITOR
void d_strupr(std::array<char, PATH_MAX> &out, const std::array<char, PATH_MAX> &in)
{
	for (auto &&[i, o] : zip(in, out))
	{
		o = std::toupper(static_cast<unsigned char>(i));
		if (!o)
			break;
	}
}
#endif

std::unique_ptr<char[]> (d_strdup)(const char *str)
{
	const auto len = strlen(str) + 1;
	std::unique_ptr<char[]> newstr(new char[len]);
	memcpy(newstr.get(), str, len);
	return newstr;
}

//give a filename a new extension, won't append if strlen(dest) > 8 chars.
bool change_filename_extension(const std::span<char> dest, const char *const src, const std::span<const char, 4> ext)
{
	const char *const p = strrchr(src, '.');
	const std::size_t src_dist_to_last_dot = p ? std::distance(src, p) : strlen(src);
	if (src_dist_to_last_dot + 1 + ext.size() > dest.size())
	{
		dest.front() = 0;
		return false;	// a non-opened file is better than a bad memory access
	}
	std::snprintf(dest.data(), dest.size(), "%.*s.%s", static_cast<int>(src_dist_to_last_dot), src, ext.data());
	return true;
}

splitpath_t d_splitpath(const char *name)
{
	const char *s, *p;

	p = name;
	s = strchr(p, ':');
	if ( s != NULL ) {
		p = s+1;
	}
	s = strrchr(p, '\\');
	if ( s != NULL) {
		p = s+1;
	}

	s = strchr(p, '.');
	if (s != NULL) {
		return {p, s};
	} else
		return {};
}

int string_array_sort_func(const void *v0, const void *v1)
{
	const auto e0 = reinterpret_cast<const char *const *>(v0);
	const auto e1 = reinterpret_cast<const char *const *>(v1);
	return d_stricmp(*e0, *e1);
}

void string_array_t::add(const char *s)
{
	const auto insert_string = [this, s]{
		auto &b = this->buffer;
		b.insert(b.end(), s, s + strlen(s) + 1);
	};
	if (buffer.empty())
	{
		insert_string();
		ptr.emplace_back(&buffer.front());
		return;
	}
	const char *ob = &buffer.front();
	ptr.emplace_back(1 + &buffer.back());
	insert_string();
	if (auto d = &buffer.front() - ob)
	{
		// Update all the pointers in the pointer list
		range_for (auto &i, ptr)
			i += d;
	}
}

void string_array_t::tidy(std::size_t offset)
{
	// Sort by name, starting at offset
	auto b = std::next(ptr.begin(), offset);
	auto e = ptr.end();
#ifdef __linux__
#define comp strcmp
#else
#define comp d_stricmp
#endif
	std::sort(b, e, [](const char *sa, const char *sb) { return d_stricmp(sa, sb) < 0; });
					  
	// Remove duplicates
	// Can't do this before reallocating, otherwise it makes a mess of things (the strings in the buffer aren't ordered)
	ptr.erase(std::unique(b, e, [=](const char *sa, const char *sb) { return comp(sa, sb) == 0; }), e);
#undef comp
}

}
