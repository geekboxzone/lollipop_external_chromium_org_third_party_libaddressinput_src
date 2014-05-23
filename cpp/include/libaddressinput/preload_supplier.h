// Copyright (C) 2014 Google Inc.
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

#ifndef I18N_ADDRESSINPUT_PRELOAD_SUPPLIER_H_
#define I18N_ADDRESSINPUT_PRELOAD_SUPPLIER_H_

#include <libaddressinput/callback.h>
#include <libaddressinput/supplier.h>
#include <libaddressinput/util/basictypes.h>
#include <libaddressinput/util/scoped_ptr.h>

#include <map>
#include <set>
#include <string>

namespace i18n {
namespace addressinput {

class Downloader;
class LookupKey;
class RegionData;
class Retriever;
class Rule;
class Storage;

// An implementation of the Supplier interface that owns a Retriever object,
// through which it can load aggregated address metadata for a region when
// instructed to, creating Rule objects and caching these. It also provides
// methods to check whether metadata for a particular region is already loaded
// or in progress of being loaded.
//
// When using a PreloadSupplier, it becomes possible to do synchronous address
// validation using an asynchronous Downloader, and to have full control over
// when network access is being done.
//
// The maximum size of this cache is naturally limited to the amount of data
// available from the data server. (Currently this is less than 12,000 items of
// in total less than 2 MB of JSON data.)
class PreloadSupplier : public Supplier {
 public:
  typedef i18n::addressinput::Callback<std::string, int> Callback;

  // Takes ownership of |downloader| and |storage|. The |validation_data_url|
  // should be a URL to a service that returns address metadata aggregated per
  // region, and which |downloader| can access.
  //
  // (See the documentation for the Downloader implementation used for
  // information about what URLs are useable with that Downloader.)
  PreloadSupplier(const std::string& validation_data_url,
                  const Downloader* downloader,
                  Storage* storage);
  virtual ~PreloadSupplier();

  // Collects the metadata needed for |lookup_key| from the cache, then calls
  // |supplied|. If the metadata needed isn't found in the cache, it will call
  // the callback with status false.
  virtual void Supply(const LookupKey& lookup_key,
                      const Supplier::Callback& supplied);

  // Should be called only when IsLoaded() returns true for the region code of
  // the |lookup_key|. Can return NULL if the |lookup_key| does not correspond
  // to any rule data. The caller does not own the result.
  const Rule* GetRule(const LookupKey& lookup_key);

  // Loads all address metadata available for |region_code|. (A typical data
  // size is 10 kB. The largest is 250 kB.)
  //
  // If the rules are already in progress of being loaded, it does nothing.
  // Calls |loaded| when the loading has finished.
  void LoadRules(const std::string& region_code, const Callback& loaded);

  bool IsLoaded(const std::string& region_code) const;
  bool IsPending(const std::string& region_code) const;

  // Returns a tree of administrative subdivisions for the |region_code|.
  // Examples:
  //   US with en-US UI language.
  //   |______________________
  //   |           |          |
  //   v           v          v
  // AL:Alabama  AK:Alaska  AS:American Samoa  ...
  //
  //   KR with ko-Latn UI language.
  //   |______________________________________
  //       |               |                  |
  //       v               v                  v
  // 강원도:Gangwon  경기도:Gyeonggi  경상남도:Gyeongnam  ...
  //
  //   KR with ko-KR UI language.
  //   |_______________________________
  //       |            |              |
  //       v            v              v
  // 강원도:강원  경기도:경기  경상남도:경남  ...
  //
  // The BCP 47 |ui_language_tag| is used to choose the best supported language
  // tag for this region (assigned to |best_region_tree_language_tag|). For
  // example, Canada has both English and French names for its administrative
  // subdivisions. If the UI language is French, then the French names are used.
  // The |best_region_tree_language_tag| value may be an empty string.
  //
  // Should be called only if IsLoaded(region_code) returns true. The
  // |best_region_tree_language_tag| parameter should not be NULL.
  const RegionData& BuildRegionTree(const std::string& region_code,
                                    const std::string& ui_language_tag,
                                    std::string* best_region_tree_language_tag);

 private:
  typedef std::map<std::string, const RegionData*> LanguageRegionMap;
  typedef std::map<std::string, LanguageRegionMap*> RegionCodeDataMap;

  bool GetRuleHierarchy(const LookupKey& lookup_key, RuleHierarchy* hierarchy);
  bool IsLoadedKey(const std::string& key) const;
  bool IsPendingKey(const std::string& key) const;
  static std::string KeyFromRegionCode(const std::string& region_code);

  const scoped_ptr<const Retriever> retriever_;
  std::set<std::string> pending_;
  std::map<std::string, const Rule*> rule_cache_;
  RegionCodeDataMap region_data_cache_;

  DISALLOW_COPY_AND_ASSIGN(PreloadSupplier);
};

}  // namespace addressinput
}  // namespace i18n

#endif  // I18N_ADDRESSINPUT_PRELOAD_SUPPLIER_H_
