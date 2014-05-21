// Copyright (C) 2013 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// ValidatingStorage saves data with checksum and timestamp using
// ValidatingUtil.

#include "validating_storage.h"

#include <libaddressinput/callback.h>
#include <libaddressinput/storage.h>
#include <libaddressinput/util/basictypes.h>
#include <libaddressinput/util/scoped_ptr.h>

#include <cassert>
#include <cstddef>
#include <ctime>
#include <string>

#include "validating_util.h"

namespace i18n {
namespace addressinput {

namespace {

class Helper {
 public:
  Helper(const std::string& key,
         const ValidatingStorage::Callback& data_ready,
         const Storage& wrapped_storage)
      : data_ready_(data_ready),
        wrapped_data_ready_(BuildCallback(this, &Helper::OnWrappedDataReady)) {
    wrapped_storage.Get(key, *wrapped_data_ready_);
  }

 private:
  ~Helper() {}

  void OnWrappedDataReady(bool success,
                          const std::string& key,
                          const std::string& wrapped_data) {
    std::string data(wrapped_data);
    if (success) {
      bool is_stale = !ValidatingUtil::UnwrapTimestamp(&data, time(NULL));
      bool is_corrupted = !ValidatingUtil::UnwrapChecksum(&data);
      data_ready_(!is_corrupted && !is_stale,
                  key,
                  is_corrupted ? std::string() : data);
    } else {
      data_ready_(false, key, std::string());
    }
    delete this;
  }

  const Storage::Callback& data_ready_;
  scoped_ptr<Storage::Callback> wrapped_data_ready_;

  DISALLOW_COPY_AND_ASSIGN(Helper);
};

}  // namespace

ValidatingStorage::ValidatingStorage(Storage* storage)
    : wrapped_storage_(storage) {
  assert(wrapped_storage_ != NULL);
}

ValidatingStorage::~ValidatingStorage() {}

void ValidatingStorage::Put(const std::string& key, const std::string& data) {
  wrapped_storage_->Put(key, ValidatingUtil::Wrap(data, time(NULL)));
}

void ValidatingStorage::Get(const std::string& key,
                            const Callback& data_ready) const {
  new Helper(key, data_ready, *wrapped_storage_);
}

}  // namespace addressinput
}  // namespace i18n
