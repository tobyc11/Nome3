/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdarg.h>
#include <string>

namespace tc
{

// These printf-like functions are implemented in terms of vsnprintf, so they
// use the same attribute for compile-time format string checking.

// Returns a string corresponding to printf-like formatting of the arguments.
#ifdef _MSC_VER
std::string StringPrintf(const char* fmt, ...);
#else
std::string StringPrintf(const char* fmt, ...) __attribute__((__format__(__printf__, 1, 2)));
#endif

// Appends a printf-like formatting of the arguments to 'dst'.
#ifdef _MSC_VER
void StringAppendF(std::string* dst, const char* fmt, ...);
#else
void StringAppendF(std::string* dst, const char* fmt, ...)
__attribute__((__format__(__printf__, 2, 3)));
#endif

// Appends a printf-like formatting of the arguments to 'dst'.
#ifdef _MSC_VER
void StringAppendV(std::string* dst, const char* format, va_list ap);
#else
void StringAppendV(std::string* dst, const char* format, va_list ap)
__attribute__((__format__(__printf__, 2, 0)));
#endif

} // namespace tc
