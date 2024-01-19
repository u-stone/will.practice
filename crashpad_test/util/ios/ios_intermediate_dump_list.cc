﻿// Copyright 2021 The Crashpad Authors. All rights reserved.
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

#include "util/ios/ios_intermediate_dump_list.h"

namespace crashpad {
namespace internal {

IOSIntermediateDumpList::IOSIntermediateDumpList() : list_() {}

IOSIntermediateDumpList::~IOSIntermediateDumpList() {}

IOSIntermediateDumpObject::Type IOSIntermediateDumpList::GetType() const {
  return Type::kList;
}

}  // namespace internal
}  // namespace crashpad