/*
 * Copyright (c) 2019-2022 ExpoLab, UC Davis
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "chain/state/chain_state.h"

#include <glog/logging.h>

namespace resdb {

ChainState::ChainState(std::unique_ptr<Storage> storage)
    : storage_(std::move(storage)) {}

Storage* ChainState::GetStorage() {
  return storage_ ? storage_.get() : nullptr;
}

int ChainState::SetValue(const std::string& key, const std::string& value) {
  if (storage_) {
    return storage_->SetValue(key, value);
  }
  kv_map_[key] = value;
  return 0;
}

std::string ChainState::GetValue(const std::string& key) {
  auto search = kv_map_.find(key);
  if (search != kv_map_.end())
    return search->second;
  else {
    if (storage_) {
      std::string value = storage_->GetValue(key);
      kv_map_[key] = value;
    }
    return "";
  }
}

std::string ChainState::GetAllValues(void) {
  if (storage_) {
    return storage_->GetAllValues();
  }
  std::string values = "[";
  bool first_iteration = true;
  for (auto kv : kv_map_) {
    if (!first_iteration) values.append(",");
    first_iteration = false;
    values.append(kv.second);
  }
  values.append("]");
  return values;
}

std::string ChainState::GetRange(const std::string& min_key,
                                 const std::string& max_key) {
  if (storage_) {
    return storage_->GetRange(min_key, max_key);
  }
  std::string values = "[";
  bool first_iteration = true;
  for (auto kv : kv_map_) {
    if (kv.first >= min_key && kv.first <= max_key) {
      if (!first_iteration) values.append(",");
      first_iteration = false;
      values.append(kv.second);
    }
  }
  values.append("]");
  return values;
}

}  // namespace resdb
