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
#include "parser/unmanm.h"
#include "parser/stream.h"
#include "parser/utils.h"

namespace veles {
namespace parser {

static void parseBlock(const dbif::ObjectHandle& blob, StreamParser* parser) {
  uint32_t label = getData(blob, parser->pos(), data::Repacker{data::Endian::LITTLE, 8, 32}, 1).element64();

  parser->startChunk("manm_data", QString("Unknown%1").arg(label));
  parser->getLe32("label");
  switch (label) {
    case 0x0001:
      parser->getLe32("time_start");
      parser->getFloat32Le("posX");
      parser->getFloat32Le("posY");
      parser->getFloat32Le("posZ");
      break;
    case 0x0002:
      parser->getLe32("time_start");
      parser->getFloat32Le("unknown");
      parser->getFloat32Le("unknown");
      parser->getFloat32Le("unknown");
      parser->getFloat32Le("unknown");
      break;
    case 0x0003:
      parser->getLe32("time_start");
      parser->getFloat32Le("unknown");
      parser->getFloat32Le("unknown");
      parser->getFloat32Le("unknown");
      break;
  }

  parser->endChunk();
}

static void parseBlockObject(const dbif::ObjectHandle& blob, StreamParser* parser) {
  uint32_t size;
  uint64_t end;

  parser->startChunk("manm_object", "object");
  parser->getLe32("label");
  size = parser->getLe32("size");
  end = parser->pos() + size - 8;
  parser->getLe32("translation_cnt");
  parser->getLe32("rotation_cnt");
  parser->getLe32("unknown3_cnt");
  parser->getLe32("unknown");
  parser->getLe32("time_duration");
  parser->getLe32("unknown");
  parser->getBytesUntil("name", '\0');
  while (parser->pos() < end) {
    parseBlock(blob, parser);
  }
  parser->endChunk();
}

void unmanmFileBlob(const dbif::ObjectHandle& blob, uint64_t start,
                   const dbif::ObjectHandle& parent_chunk) {
  StreamParser parser(blob, start, parent_chunk);

  parser.startChunk("manm_header", "header");
  parser.getBytes("sig", 4);
  parser.getLe32("unk");
  parser.getLe32("object_children");
  parser.getLe32("unk");
  parser.endChunk();

  while (parser.bytesLeft() >= 8) {
    parseBlockObject(blob, &parser);
  }

}

}  // namespace parser
}  // namespace veles

