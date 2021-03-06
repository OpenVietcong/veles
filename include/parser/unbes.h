/*
 * Copyright 2018 Jan Havran
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#pragma once

#include "data/bindata.h"
#include "dbif/types.h"
#include "parser/parser.h"

namespace veles {
namespace parser {

void unbesFileBlob(
    const dbif::ObjectHandle& blob, uint64_t start = 0,
    const dbif::ObjectHandle& parent_chunk = dbif::ObjectHandle());

class BesParser : public Parser {
 public:
  BesParser() : Parser("bes", data::BinData(8, {'B', 'E', 'S', 0x00})) {}
  void parse(const dbif::ObjectHandle& blob, uint64_t start,
             const dbif::ObjectHandle& parent_chunk) override {
    unbesFileBlob(blob, start, parent_chunk);
  }
};

}  // namespace parser
}  // namespace veles
