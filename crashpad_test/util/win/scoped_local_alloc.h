﻿// Copyright 2015 The Crashpad Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CRASHPAD_UTIL_WIN_SCOPED_LOCAL_ALLOC_H_
#define CRASHPAD_UTIL_WIN_SCOPED_LOCAL_ALLOC_H_

#include <windows.h>

#include "base/scoped_generic.h"

namespace crashpad {

namespace internal {

struct LocalAllocTraits {
  static HLOCAL InvalidValue() { return nullptr; }
  static void Free(HLOCAL mem);
};

}  // namespace internal

using ScopedLocalAlloc =
    base::ScopedGeneric<HLOCAL, internal::LocalAllocTraits>;

}  // namespace crashpad

#endif  // CRASHPAD_UTIL_WIN_SCOPED_LOCAL_ALLOC_H_
