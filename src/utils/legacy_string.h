// <string> Forward declarations -*- C++ -*-

// Copyright (C) 2001-2024 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file bits/stringfwd.h
 *  This is an internal header file, included by other library headers.
 *  Do not attempt to use it directly. @headername{string}
 */

//
// ISO C++ 14882: 21 Strings library
//

#pragma once

#pragma GCC system_header

#include <bits/c++config.h>

namespace std _GLIBCXX_VISIBILITY(default)
{
 /**
   * @defgroup allocators Allocators
   * @ingroup memory
   *
   * Classes encapsulating memory operations.
   *
   * @{
   */

  // Included in freestanding as a libstdc++ extension.
  template<typename>
    class allocator;

  template<>
    class allocator<void>;

#if __cplusplus >= 201103L
  /// Declare uses_allocator so it can be specialized in `<queue>` etc.
  template<typename, typename>
    struct uses_allocator;

  template<typename>
    struct allocator_traits;
#endif

  /**
   *  @defgroup strings Strings
   *
   *  @{
  */

  template<class _CharT> struct char_traits;

  template<> struct char_traits<char>;

  template<> struct char_traits<wchar_t>;

#ifdef _GLIBCXX_USE_CHAR8_T
  template<> struct char_traits<char8_t>;
#endif

#if __cplusplus >= 201103L
  template<> struct char_traits<char16_t>;
  template<> struct char_traits<char32_t>;
#endif

  template<typename _CharT, typename _Traits = char_traits<_CharT>,
           typename _Alloc = allocator<_CharT> >
    class legacy_basic_string;

  /// A string of @c char
  typedef legacy_basic_string<char>    legacy_string;   

  /// A string of @c wchar_t
  typedef legacy_basic_string<wchar_t> legacy_wstring;   

#ifdef _GLIBCXX_USE_CHAR8_T
  /// A string of @c char8_t
  typedef legacy_basic_string<char8_t> legacy_u8string;
#endif

#if __cplusplus >= 201103L
  /// A string of @c char16_t
  typedef legacy_basic_string<char16_t> legacy_u16string; 

  /// A string of @c char32_t
  typedef legacy_basic_string<char32_t> legacy_u32string; 
#endif

  /** @}  */
} // namespace std
