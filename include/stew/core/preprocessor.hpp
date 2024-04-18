/*
 * Copyright (C) 2023 bitWelder
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see
 * <http://www.gnu.org/licenses/>
 */

#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

//--------------------------------------------------------------------------------------------------
// Identify the cpp compiler
#ifndef PREPROCESSOR_CPP
#   ifdef _MSVC_LANG
#       if _MSVC_LANG > __cplusplus
#           define PREPROCESSOR_CPP _MSVC_LANG
#       endif
#   endif
#   ifndef PREPROCESSOR_CPP
#       define PREPROCESSOR_CPP __cplusplus
#   endif
#   if PREPROCESSOR_CPP >= 202900L
#       undef PREPROCESSOR_CPP
#       define PREPROCESSOR_CPP 29
#   elif PREPROCESSOR_CPP >= 202600L
#       undef PREPROCESSOR_CPP
#       define PREPROCESSOR_CPP 26
#   elif PREPROCESSOR_CPP >= 202302L
#       undef PREPROCESSOR_CPP
#       define PREPROCESSOR_CPP 23
#   elif PREPROCESSOR_CPP >= 202002L
#       undef PREPROCESSOR_CPP
#       define PREPROCESSOR_CPP 20
#   elif PREPROCESSOR_CPP >= 201703L
#       undef PREPROCESSOR_CPP
#       define PREPROCESSOR_CPP 17
#   elif PREPROCESSOR_CPP >= 201402L
#       undef PREPROCESSOR_CPP
#       define PREPROCESSOR_CPP 14
#   elif PREPROCESSOR_CPP >= 201103L
#       undef PREPROCESSOR_CPP
#       define PREPROCESSOR_CPP 11
#   else
#       undef PREPROCESSOR_CPP
#       define PREPROCESSOR_CPP 0
#   endif
#endif

//--------------------------------------------------------------------------------------------------
// Identify cpp compiler type
#ifndef PREPROCESSOR_CPP_CLANG
#   ifdef __clang__
#       if defined(__apple_build_version__)
#           /* http://en.wikipedia.org/wiki/Xcode#Toolchain_Versions */
#           if __apple_build_version__ >= 7000053
#               define PREPROCESSOR_CPP_CLANG 306
#           elif __apple_build_version__ >= 6000051
#               define PREPROCESSOR_CPP_CLANG 305
#           elif __apple_build_version__ >= 5030038
#               define PREPROCESSOR_CPP_CLANG 304
#           elif __apple_build_version__ >= 5000275
#               define PREPROCESSOR_CPP_CLANG 303
#           elif __apple_build_version__ >= 4250024
#               define PREPROCESSOR_CPP_CLANG 302
#           elif __apple_build_version__ >= 3180045
#               define PREPROCESSOR_CPP_CLANG 301
#           elif __apple_build_version__ >= 2111001
#               define PREPROCESSOR_CPP_CLANG 300
#           else
#               error "Unknown Apple Clang version"
#           endif
#           define PREPROCESSOR_LONG_SYNONIM_OF_UINT64
#       else
#           define PREPROCESSOR_CPP_CLANG ((__clang_major__ * 100) + __clang_minor__)
#       endif
#   else
#       define PREPROCESSOR_CPP_CLANG 0
#   endif
#endif
#ifndef PREPROCESSOR_CPP_MSVC
#   if defined(_MSC_VER) && !PREPROCESSOR_CPP_CLANG
#       define PREPROCESSOR_CPP_MSVC _MSC_VER
#   else
#       define PREPROCESSOR_CPP_MSVC 0
#   endif
#endif
#ifndef PREPROCESSOR_CPP_GCC
#   if defined(__GNUC__) && !PREPROCESSOR_CPP_CLANG
#       define PREPROCESSOR_CPP_GCC __GNUC__
#   else
#       define PREPROCESSOR_CPP_GCC 0
#   endif
#endif

//--------------------------------------------------------------------------------------------------
// Utilities
#ifndef DISABLE_COPY
#   define DISABLE_COPY(Class) \
		Class(const Class&) = delete;\
		Class& operator=(const Class&) = delete
#endif

#ifndef DISABLE_MOVE
#   define DISABLE_MOVE(Class) \
		Class(Class&&) = delete; \
		Class& operator=(Class&&) = delete
#endif

#ifndef ATTRIBUTE
#   if PREPROCESSOR_CPP_CLANG || PREPROCESSOR_CPP_GCC
#       define ATTRIBUTE(...)  __attribute__((__VA_ARGS__))
#   else
#       define ATTRIBUTE(...)
#   endif
#endif

#ifndef HAS_CPP_ATTRIBUTE
#   ifdef __has_cpp_attribute
#       define HAS_CPP_ATTRIBUTE(attr)  __has_cpp_attribute(attr)
#   else
#       define HAS_CPP_ATTRIBUTE(attr)  0
#   endif
#endif

#ifndef IMPORT_API
#   if PREPROCESSOR_CPP_MSVC
#       define IMPORT_API   __declspec(dllimport)
#   elif PREPROCESSOR_CPP_CLANG || PREPROCESSOR_CPP_GCC
#       define IMPORT_API   ATTRIBUTE(visibility("default"))
#   else
#       define IMPORT_API
#   endif
#endif

#ifndef EXPORT_API
#   if PREPROCESSOR_CPP_MSVC
#       define EXPORT_API   __declspec(dllexport)
#   elif PREPROCESSOR_CPP_CLANG || PREPROCESSOR_CPP_GCC
#       define EXPORT_API   ATTRIBUTE(visibility("default"))
#   else
#       define EXPORT_API
#   endif
#endif

#ifndef TEMPLATE_IMPORT_API
#   define TEMPLATE_IMPORT_API
#endif

#ifndef TEMPLATE_EXPORT_API
#   if PREPROCESSOR_CPP_MSVC
#       define TEMPLATE_EXPORT_API   __declspec(dllexport)
#   else
#       define TEMPLATE_EXPORT_API
#   endif
#endif

#ifdef CONFIG_LIBRARY
#   define PROJECT_API      EXPORT_API
#   define TEMPLATE_API     TEMPLATE_EXPORT_API
#else
#   define PROJECT_API      IMPORT_API
#   define TEMPLATE_API     TEMPLATE_IMPORT_API
#endif

#ifndef MAYBE_UNUSED
#   define MAYBE_UNUSED(x)  (void)(x)
#endif

#endif // PREPROCESSOR_HPP
