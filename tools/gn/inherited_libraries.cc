// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/gn/inherited_libraries.h"

#include "base/containers/adapters.h"
#include "tools/gn/target.h"

InheritedLibraries::InheritedLibraries() = default;

InheritedLibraries::~InheritedLibraries() = default;

std::vector<const Target*> InheritedLibraries::GetOrdered() const {
  std::vector<const Target*> result;
  result.resize(map_.size());

  // The indices in the map should be from 0 to the number of items in the
  // map, so insert directly into the result (with some sanity checks).
  for (const auto& pair : map_) {
    size_t rindex = pair.second.rindex;
    size_t index = (result.size() - 1) - rindex;
    DCHECK(rindex < result.size());
    DCHECK(!result[index]);
    result[index] = pair.first;
  }

  return result;
}

std::vector<std::pair<const Target*, bool>>
InheritedLibraries::GetOrderedAndPublicFlag() const {
  std::vector<std::pair<const Target*, bool>> result;
  result.resize(map_.size());

  for (const auto& pair : map_) {
    size_t rindex = pair.second.rindex;
    size_t index = (result.size() - 1) - rindex;
    DCHECK(rindex < result.size());
    DCHECK(!result[index].first);
    result[index] = std::make_pair(pair.first, pair.second.is_public);
  }

  return result;
}

void InheritedLibraries::Append(const Target* target, bool is_public) {
  // Try to insert a new node.
  auto insert_result =
      map_.insert(std::make_pair(target, Node(map_.size(), is_public)));

  if (!insert_result.second) {
    // Element already present, insert failed and insert_result indicates the
    // old one. The old one may need to have its public flag updated.
    if (is_public) {
      Node& existing_node = insert_result.first->second;
      existing_node.is_public = true;
    }
  }
}

void InheritedLibraries::AppendInherited(const InheritedLibraries& other,
                                         bool is_public) {
  // Append all items in order, mark them public only if the're already public
  // and we're adding them publically.
  auto v = other.GetOrderedAndPublicFlag();
  for (const auto& cur : base::Reversed(v))
    Append(cur.first, is_public && cur.second);
}

void InheritedLibraries::AppendFinal(const InheritedLibraries& other,
                                     bool is_public) {
  // Append only final items in order, mark them public only if the're already
  // public and we're adding them publically.
  auto v = other.GetOrderedAndPublicFlag();
  for (const auto& cur : base::Reversed(v))
    if (cur.first->IsFinal())
      Append(cur.first, is_public && cur.second);
}

void InheritedLibraries::AppendPublicSharedLibraries(
    const InheritedLibraries& other,
    bool is_public) {
  auto v = other.GetOrderedAndPublicFlag();
  for (const auto& cur : base::Reversed(v)) {
    if (cur.first->output_type() == Target::SHARED_LIBRARY && cur.second)
      Append(cur.first, is_public);
  }
}
