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
#include "parser/unbes.h"

#include <map>

#include "parser/stream.h"
#include "parser/utils.h"

namespace veles {
namespace parser {

typedef enum e_BES_BL {
  BES_BL_OBJECT     = 0x0001,
  BES_BL_UNK30      = 0x0030,
  BES_BL_MESH       = 0x0031,
  BES_BL_VERTICES   = 0x0032,
  BES_BL_FACES      = 0x0033,
  BES_BL_PROPERTIES = 0x0034,
  BES_BL_UNK35      = 0x0035,
  BES_BL_UNK36      = 0x0036,
  BES_BL_UNK38      = 0x0038,
  BES_BL_USER_INFO  = 0x0070,
  BES_BL_MATERIAL   = 0x1000,
  BES_BL_BITMAP     = 0x1001,
  BES_BL_PTEROMAT   = 0x1002,
} BES_BL;

typedef std::map<BES_BL, QString> besBlockMap_t;
besBlockMap_t besBlockMap = {
  {BES_BL_OBJECT,     "Object"},
  {BES_BL_UNK30,      "Unk30"},
  {BES_BL_MESH,       "Mesh"},
  {BES_BL_VERTICES,   "Vertices"},
  {BES_BL_FACES,      "Faces"},
  {BES_BL_PROPERTIES, "Properties"},
  {BES_BL_UNK35,      "Unk35"},
  {BES_BL_UNK36,      "Unk36"},
  {BES_BL_UNK38,      "Unk38"},
  {BES_BL_USER_INFO,  "UserInfo"},
  {BES_BL_MATERIAL,   "Material"},
  {BES_BL_BITMAP,     "Bitmap"},
  {BES_BL_PTEROMAT,   "PteroMat"},
};

static void parseBlock(const dbif::ObjectHandle& blob, StreamParser* parser);

static void parseSubBlocks(const dbif::ObjectHandle& blob, StreamParser* parser, uint64_t end) {
  while (parser->pos() + 8 < end) {
    parseBlock(blob, parser);
  }
}

static void parseBlockObject(const dbif::ObjectHandle& blob, StreamParser* parser, uint64_t end) {
  uint32_t nameLen;

  parser->getLe32("object_children");
  nameLen = parser->getLe32("name_len");
  parser->getBytes("name", nameLen);
  parseSubBlocks(blob, parser, end);

  if (parser->pos() < end)
    parser->skip(end - parser->pos());
}

static void parseBlockUnk30(const dbif::ObjectHandle& blob, StreamParser* parser, uint64_t end) {
  parser->getLe32("mesh_children");
  parseSubBlocks(blob, parser, end);

  if (parser->pos() < end)
    parser->skip(end - parser->pos());
}

static void parseBlockMesh(const dbif::ObjectHandle& blob, StreamParser* parser, uint64_t end) {
  parser->getLe32("material");
  parseSubBlocks(blob, parser, end);

  if (parser->pos() < end)
    parser->skip(end - parser->pos());
}

static void parseBlockVertices(const dbif::ObjectHandle& blob, StreamParser* parser, uint64_t end) {
  uint32_t verticesCount = parser->getLe32("vertices_count");
  uint32_t vertex_size   = parser->getLe32("vertex_size");
  parser->getLe32("unknown");

  for (uint32_t vertex = 0; vertex < verticesCount; vertex++) {
      parser->startChunk("bes_vertex", QString("Vertex[%1]").arg(vertex));
      parser->getFloat32Le("posX");
      parser->getFloat32Le("posY");
      parser->getFloat32Le("posZ");
      parser->skip(vertex_size - 12);
      parser->endChunk();
  }

  if (parser->pos() < end)
    parser->skip(end - parser->pos());
}

static void parseBlockFaces(const dbif::ObjectHandle& blob, StreamParser* parser, uint64_t end) {
  uint32_t facesCount  = parser->getLe32("faces_count");

  for (uint32_t face = 0; face < facesCount; face++) {
      parser->startChunk("bes_face", QString("Face[%1]").arg(face));
      parser->getLe32("vertexA");
      parser->getLe32("vertexB");
      parser->getLe32("vertexC");
      parser->endChunk();
  }

  if (parser->pos() < end)
    parser->skip(end - parser->pos());
}

static void parseBlockProperties(const dbif::ObjectHandle& blob, StreamParser* parser, uint64_t end) {
  uint32_t textLen = parser->getLe32("tex_len");
  parser->getBytes("text", textLen);

  if (parser->pos() < end)
    parser->skip(end - parser->pos());
}

static void parseBlockUnk35(const dbif::ObjectHandle& blob, StreamParser* parser, uint64_t end) {
  parser->skip(end - parser->pos());
}

static void parseBlockUnk36(const dbif::ObjectHandle& blob, StreamParser* parser, uint64_t end) {
  parser->getLe32("unknown");

  if (parser->pos() < end)
    parser->skip(end - parser->pos());
}

static void parseBlockUnk38(const dbif::ObjectHandle& blob, StreamParser* parser, uint64_t end) {
  parser->skip(end - parser->pos());
}

static void parseBlockUserInfo(const dbif::ObjectHandle& blob, StreamParser* parser, uint64_t end) {
  uint32_t nameLen = parser->getLe32("name_len");
  uint32_t commentLen = parser->getLe32("comment_len");
  parser->getLe32("unknown");
  parser->getBytes("name", nameLen);
  parser->getBytes("comment", commentLen);

  if (parser->pos() < end)
    parser->skip(end - parser->pos());
}

static void parseBlockMaterial(const dbif::ObjectHandle& blob, StreamParser* parser, uint64_t end) {
  parser->getLe32("material_children");
  parseSubBlocks(blob, parser, end);

  if (parser->pos() < end)
    parser->skip(end - parser->pos());
}

static void parseBlockBitmap(const dbif::ObjectHandle& blob, StreamParser* parser, uint64_t end) {
  parser->getLe32("unknown");
  parser->getLe32("unknown");
  parser->getLe32("type");

  if (parser->pos() < end)
    parser->skip(end - parser->pos());
}

static void parseBlockPteroMat(const dbif::ObjectHandle& blob, StreamParser* parser, uint64_t end) {
  uint32_t nameLen;

  parser->getLe32("sides");
  parser->getLe32("type");
  parser->getBytes("collision_mat", 4);
  parser->getLe32("unknown");
  parser->getBytes("vegetation", 4);
  nameLen = parser->getLe32("name_len");
  parser->getBytes("name", nameLen);

  if (parser->pos() < end)
    parser->skip(end - parser->pos());
}

static void parseBlock(const dbif::ObjectHandle& blob, StreamParser* parser) {
  besBlockMap_t::iterator iter;
  e_BES_BL label = (e_BES_BL) ((uint32_t) getData(blob, parser->pos(),
                   data::Repacker{data::Endian::LITTLE, 8, 32}, 1).element64());
  uint32_t size  = getData(blob, parser->pos() + 4,
                   data::Repacker{data::Endian::LITTLE, 8, 32}, 1).element64();
  uint64_t end   = parser->pos() + size;

  iter = besBlockMap.find(label);
  if (iter != besBlockMap.end())
    parser->startChunk("bes_block", iter->second);
  else
    parser->startChunk("bes_block", "Unknown");

  parser->getLe32("label");
  parser->getLe32("size");

  switch (label) {
    case BES_BL_OBJECT:
      parseBlockObject(blob, parser, end);
      break;
    case BES_BL_UNK30:
      parseBlockUnk30(blob, parser, end);
      break;
    case BES_BL_MESH:
      parseBlockMesh(blob, parser, end);
      break;
    case BES_BL_VERTICES:
      parseBlockVertices(blob, parser, end);
      break;
    case BES_BL_FACES:
      parseBlockFaces(blob, parser, end);
      break;
    case BES_BL_PROPERTIES:
      parseBlockProperties(blob, parser, end);
      break;
    case BES_BL_UNK35:
      parseBlockUnk35(blob, parser, end);
      break;
    case BES_BL_UNK36:
      parseBlockUnk36(blob, parser, end);
      break;
    case BES_BL_UNK38:
      parseBlockUnk38(blob, parser, end);
      break;
    case BES_BL_USER_INFO:
      parseBlockUserInfo(blob, parser, end);
      break;
    case BES_BL_MATERIAL:
      parseBlockMaterial(blob, parser, end);
      break;
    case BES_BL_BITMAP:
      parseBlockBitmap(blob, parser, end);
      break;
    case BES_BL_PTEROMAT:
      parseBlockPteroMat(blob, parser, end);
      break;
    default:
      parser->skip(size);
      break;
  }
  parser->endChunk();
}

void unbesFileBlob(const dbif::ObjectHandle& blob, uint64_t start,
                   const dbif::ObjectHandle& parent_chunk) {
  StreamParser parser(blob, start, parent_chunk);

  parser.startChunk("bes_header", "header");
  parser.getBytes("sig", 4);
  parser.getBytes("ver", 5);
  parser.getLe32("unk");
  parser.getBytes("ver", 3);
  parser.endChunk();

  parser.startChunk("bes_preview", "preview");
  parser.getBytes("preview", 12288);
  parser.endChunk();

  while (parser.bytesLeft() >= 8) {
    parseBlock(blob, &parser);
  }
}

}  // namespace parser
}  // namespace veles

