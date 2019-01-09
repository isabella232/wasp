//
// Copyright 2019 WebAssembly Community Group participants
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

#ifndef WASP_BINARY_READ_READ_VAR_INT_H_
#define WASP_BINARY_READ_READ_VAR_INT_H_

#include <type_traits>

#include "wasp/base/features.h"
#include "wasp/base/format.h"
#include "wasp/base/formatters.h"
#include "wasp/base/optional.h"
#include "wasp/base/span.h"
#include "wasp/base/string_view.h"
#include "wasp/base/types.h"
#include "wasp/binary/errors_context_guard.h"
#include "wasp/binary/read/macros.h"
#include "wasp/binary/read/read.h"
#include "wasp/binary/read/read_u8.h"

namespace wasp {
namespace binary {

template <typename S>
S SignExtend(typename std::make_unsigned<S>::type x, int N) {
  constexpr size_t kNumBits = sizeof(S) * 8;
  return static_cast<S>(x << (kNumBits - N - 1)) >> (kNumBits - N - 1);
}

template <typename T, typename Errors>
optional<T> ReadVarInt(SpanU8* data,
                       const Features& features,
                       Errors& errors,
                       string_view desc) {
  using U = typename std::make_unsigned<T>::type;
  constexpr bool is_signed = std::is_signed<T>::value;
  constexpr int kMaxBytes = (sizeof(T) * 8 + 6) / 7;
  constexpr int kUsedBitsInLastByte = sizeof(T) * 8 - 7 * (kMaxBytes - 1);
  constexpr int kMaskBits = kUsedBitsInLastByte - (is_signed ? 1 : 0);
  constexpr u8 kMask = ~((1 << kMaskBits) - 1);
  constexpr u8 kOnes = kMask & 0x7f;

  ErrorsContextGuard<Errors> guard{errors, *data, desc};

  U result{};
  for (int i = 0;;) {
    WASP_TRY_READ(byte, Read<u8>(data, features, errors));

    const int shift = i * 7;
    result |= U(byte & 0x7f) << shift;

    if (++i == kMaxBytes) {
      if ((byte & kMask) == 0 || (is_signed && (byte & kMask) == kOnes)) {
        return static_cast<T>(result);
      }
      const u8 zero_ext = byte & ~kMask & 0x7f;
      const u8 one_ext = (byte | kOnes) & 0x7f;
      if (is_signed) {
        errors.OnError(
            *data, format("Last byte of {} must be sign "
                          "extension: expected {:#2x} or {:#2x}, got {:#2x}",
                          desc, zero_ext, one_ext, byte));
      } else {
        errors.OnError(*data, format("Last byte of {} must be zero "
                                     "extension: expected {:#2x}, got {:#2x}",
                                     desc, zero_ext, byte));
      }
      return nullopt;
    } else if ((byte & 0x80) == 0) {
      return is_signed ? SignExtend<T>(result, 6 + shift) : result;
    }
  }
}

}  // namespace binary
}  // namespace wasp

#endif  // WASP_BINARY_READ_READ_VAR_INT_H_