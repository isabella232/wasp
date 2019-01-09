//
// Copyright 2018 WebAssembly Community Group participants
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
//

#include "wasp/binary/read/read_code.h"

#include "gtest/gtest.h"

#include "src/binary/read_test_utils.h"
#include "src/binary/test_utils.h"

using namespace ::wasp;
using namespace ::wasp::binary;
using namespace ::wasp::binary::test;

TEST(ReaderTest, Code) {
  // Empty body. This will fail validation, but can still be read.
  ExpectRead<Code>(Code{{}, MakeExpression("")}, MakeSpanU8("\x01\x00"));

  // Smallest valid empty body.
  ExpectRead<Code>(Code{{}, MakeExpression("\x0b")},
                   MakeSpanU8("\x02\x00\x0b"));

  // (func
  //   (local i32 i32 i64 i64 i64)
  //   (nop))
  ExpectRead<Code>(Code{{Locals{2, ValueType::I32}, Locals{3, ValueType::I64}},
                        MakeExpression("\x01\x0b")},
                   MakeSpanU8("\x07\x02\x02\x7f\x03\x7e\x01\x0b"));
}

TEST(ReaderTest, Code_PastEnd) {
  ExpectReadFailure<Code>(
      {{0, "code"}, {0, "length"}, {0, "Unable to read u8"}}, MakeSpanU8(""));

  ExpectReadFailure<Code>({{0, "code"}, {1, "Length extends past end: 1 > 0"}},
                          MakeSpanU8("\x01"));

  ExpectReadFailure<Code>(
      {{0, "code"}, {1, "locals vector"}, {2, "Count extends past end: 1 > 0"}},
      MakeSpanU8("\x01\x01"));
}