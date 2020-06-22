//
// Copyright 2020 WebAssembly Community Group participants
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

#include "wasp/base/utf8.h"

#include <cassert>

#include "gtest/gtest.h"

using namespace ::wasp;

namespace {

void assert_is_valid_utf8(bool expected,
                          int length,
                          int cu0 = 0,
                          int cu1 = 0,
                          int cu2 = 0,
                          int cu3 = 0) {
  assert(length <= 4);
  char buf[4] = {static_cast<char>(cu0), static_cast<char>(cu1),
                 static_cast<char>(cu2), static_cast<char>(cu3)};
  if (expected) {
    // Make sure it fails if there are continuation bytes past the end of the
    // string.
    for (int bad_length = 1; bad_length < length; ++bad_length) {
      ASSERT_FALSE(IsValidUtf8(string_view(buf, bad_length)))
          << cu0 << ", " << cu1 << ", " << cu2 << ", " << cu3;
    }
  }

  ASSERT_TRUE(expected == IsValidUtf8(string_view(buf, length)))
      << cu0 << ", " << cu1 << ", " << cu2 << ", " << cu3;
}

bool is_in_range(int x, int low, int high) {
  return x >= low && x < high;
}

}  // end anonymous namespace

#define FOR_RANGE(var, low, high) for (int var = low; var < high; var++)
#define FOR_EACH_BYTE(var) FOR_RANGE(var, 0, 0x100)
#define FOR_EACH_BYTE_SKIP(var, skip) \
  for (int var = 0; var < 0x100; var += skip)

TEST(Utf8Test, valid_empty) {
  assert_is_valid_utf8(true, 0);
}

TEST(Utf8Test, valid_1_byte) {
  FOR_RANGE(cu0, 0, 0x80) { assert_is_valid_utf8(true, 1, cu0); }
}

TEST(Utf8Test, invalid_continuation_bytes) {
  FOR_RANGE(cu0, 0x80, 0xc0) { assert_is_valid_utf8(false, 1, cu0); }
}

TEST(Utf8Test, invalid_2_byte) {
  FOR_RANGE(cu0, 0xc0, 0xc2) { assert_is_valid_utf8(false, 1, cu0); }
}

TEST(Utf8Test, valid_2_bytes) {
  FOR_RANGE(cu0, 0xc2, 0xe0) {
    FOR_EACH_BYTE(cu1) {
      bool is_valid = is_in_range(cu1, 0x80, 0xc0);
      assert_is_valid_utf8(is_valid, 2, cu0, cu1);
    }
  }
}

TEST(Utf8Test, valid_3_bytes_e0) {
  int cu0 = 0xe0;
  FOR_EACH_BYTE(cu1) {
    FOR_EACH_BYTE(cu2) {
      bool is_valid =
          is_in_range(cu1, 0xa0, 0xc0) && is_in_range(cu2, 0x80, 0xc0);
      assert_is_valid_utf8(is_valid, 3, cu0, cu1, cu2);
    }
  }
}

TEST(Utf8Test, valid_3_bytes) {
  FOR_RANGE(cu0, 0xe1, 0xf0) {
    // Handle 0xed in valid_3_bytes_ed.
    if (cu0 == 0xed) {
      continue;
    }

    FOR_EACH_BYTE(cu1) {
      FOR_EACH_BYTE(cu2) {
        bool is_valid =
            is_in_range(cu1, 0x80, 0xc0) && is_in_range(cu2, 0x80, 0xc0);
        assert_is_valid_utf8(is_valid, 3, cu0, cu1, cu2);
      }
    }
  }
}

TEST(Utf8Test, valid_3_bytes_ed) {
  int cu0 = 0xed;
  FOR_EACH_BYTE(cu1) {
    FOR_EACH_BYTE(cu2) {
      bool is_valid =
          is_in_range(cu1, 0x80, 0xa0) && is_in_range(cu2, 0x80, 0xc0);
      assert_is_valid_utf8(is_valid, 3, cu0, cu1, cu2);
    }
  }
}

TEST(Utf8Test, valid_4_bytes_f0) {
  int cu0 = 0xf0;
  FOR_EACH_BYTE_SKIP(cu1, 4) {
    FOR_EACH_BYTE_SKIP(cu2, 4) {
      FOR_EACH_BYTE_SKIP(cu3, 4) {
        bool is_valid = is_in_range(cu1, 0x90, 0xc0) &&
                        is_in_range(cu2, 0x80, 0xc0) &&
                        is_in_range(cu3, 0x80, 0xc0);
        assert_is_valid_utf8(is_valid, 4, cu0, cu1, cu2, cu3);
      }
    }
  }
}

TEST(Utf8Test, valid_4_bytes) {
  FOR_RANGE(cu0, 0xf1, 0xf4) {
    FOR_EACH_BYTE_SKIP(cu1, 4) {
      FOR_EACH_BYTE_SKIP(cu2, 4) {
        FOR_EACH_BYTE_SKIP(cu3, 4) {
          bool is_valid = is_in_range(cu1, 0x80, 0xc0) &&
                          is_in_range(cu2, 0x80, 0xc0) &&
                          is_in_range(cu3, 0x80, 0xc0);
          assert_is_valid_utf8(is_valid, 4, cu0, cu1, cu2, cu3);
        }
      }
    }
  }
}

TEST(Utf8Test, valid_4_bytes_f4) {
  int cu0 = 0xf4;
  FOR_EACH_BYTE_SKIP(cu1, 4) {
    FOR_EACH_BYTE_SKIP(cu2, 4) {
      FOR_EACH_BYTE_SKIP(cu3, 4) {
        bool is_valid = is_in_range(cu1, 0x80, 0x90) &&
                        is_in_range(cu2, 0x80, 0xc0) &&
                        is_in_range(cu3, 0x80, 0xc0);
        assert_is_valid_utf8(is_valid, 4, cu0, cu1, cu2, cu3);
      }
    }
  }
}

TEST(Utf8Test, invalid_4_bytes) {
  FOR_RANGE(cu0, 0xf5, 0x100) {
    assert_is_valid_utf8(false, 4, cu0, 0x80, 0x80, 0x80);
  }
}
