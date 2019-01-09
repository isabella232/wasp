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

#include "wasp/binary/formatters.h"

#include "wasp/base/formatters.h"
#include "wasp/base/macros.h"
#include "wasp/binary/value_type.h"

namespace fmt {

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::ValueType>::format(
    const ::wasp::binary::ValueType& self,
    Ctx& ctx) {
  string_view result;
  switch (self) {
#define WASP_V(val, Name, str)          \
  case ::wasp::binary::ValueType::Name: \
    result = str;                       \
    break;
#include "wasp/binary/value_type.def"
#undef WASP_V
    default:
      WASP_UNREACHABLE();
  }
  return formatter<string_view>::format(result, ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::BlockType>::format(
    const ::wasp::binary::BlockType& self,
    Ctx& ctx) {
  string_view result;
  switch (self) {
#define WASP_V(val, Name, str)          \
  case ::wasp::binary::BlockType::Name: \
    result = "[" str "]";               \
    break;
#include "wasp/binary/block_type.def"
#undef WASP_V
    default:
      WASP_UNREACHABLE();
  }
  return formatter<string_view>::format(result, ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::ElementType>::format(
    const ::wasp::binary::ElementType& self,
    Ctx& ctx) {
  string_view result;
  switch (self) {
#define WASP_V(val, Name, str)            \
  case ::wasp::binary::ElementType::Name: \
    result = str;                         \
    break;
#include "wasp/binary/element_type.def"
#undef WASP_V
    default:
      WASP_UNREACHABLE();
  }
  return formatter<string_view>::format(result, ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::ExternalKind>::format(
    const ::wasp::binary::ExternalKind& self,
    Ctx& ctx) {
  string_view result;
  switch (self) {
#define WASP_V(val, Name, str)             \
  case ::wasp::binary::ExternalKind::Name: \
    result = str;                          \
    break;
#include "wasp/binary/external_kind.def"
#undef WASP_V
    default:
      WASP_UNREACHABLE();
  }
  return formatter<string_view>::format(result, ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Mutability>::format(
    const ::wasp::binary::Mutability& self,
    Ctx& ctx) {
  string_view result;
  switch (self) {
#define WASP_V(val, Name, str)           \
  case ::wasp::binary::Mutability::Name: \
    result = str;                        \
    break;
#include "wasp/binary/mutability.def"
#undef WASP_V
    default:
      WASP_UNREACHABLE();
  }
  return formatter<string_view>::format(result, ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::SectionId>::format(
    const ::wasp::binary::SectionId& self,
    Ctx& ctx) {
  string_view result;
  switch (self) {
#define WASP_V(val, Name, str)          \
  case ::wasp::binary::SectionId::Name: \
    result = str;                       \
    break;
#include "wasp/binary/section_id.def"
#undef WASP_V
    default: {
      // Special case for sections with unknown ids.
      memory_buffer buf;
      format_to(buf, "{}", static_cast<::wasp::u32>(self));
      return formatter<string_view>::format(to_string_view(buf), ctx);
    }
  }
  return formatter<string_view>::format(result, ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::NameSubsectionId>::format(
    const ::wasp::binary::NameSubsectionId& self,
    Ctx& ctx) {
  string_view result;
  switch (self) {
#define WASP_V(val, Name, str)                 \
  case ::wasp::binary::NameSubsectionId::Name: \
    result = str;                              \
    break;
#include "wasp/binary/name_subsection_id.def"
#undef WASP_V
    default:
      WASP_UNREACHABLE();
  }
  return formatter<string_view>::format(result, ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::MemArgImmediate>::format(
    const ::wasp::binary::MemArgImmediate& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{{align {}, offset {}}}", self.align_log2, self.offset);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Limits>::format(
    const ::wasp::binary::Limits& self,
    Ctx& ctx) {
  memory_buffer buf;
  if (self.max) {
    format_to(buf, "{{min {}, max {}}}", self.min, *self.max);
  } else {
    format_to(buf, "{{min {}}}", self.min);
  }
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Locals>::format(
    const ::wasp::binary::Locals& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{} ** {}", self.type, self.count);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Section>::format(
    const ::wasp::binary::Section& self,
    Ctx& ctx) {
  memory_buffer buf;
  if (self.is_known()) {
    format_to(buf, "{}", self.known());
  } else if (self.is_custom()) {
    format_to(buf, "{}", self.custom());
  } else {
    WASP_UNREACHABLE();
  }
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::KnownSection>::format(
    const ::wasp::binary::KnownSection& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{{id {}, contents {}}}", self.id, self.data);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::CustomSection>::format(
    const ::wasp::binary::CustomSection& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{{name \"{}\", contents {}}}", self.name, self.data);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::TypeEntry>::format(
    const ::wasp::binary::TypeEntry& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{}", self.type);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::FunctionType>::format(
    const ::wasp::binary::FunctionType& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{} -> {}", self.param_types, self.result_types);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::TableType>::format(
    const ::wasp::binary::TableType& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{} {}", self.limits, self.elemtype);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::MemoryType>::format(
    const ::wasp::binary::MemoryType& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{}", self.limits);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::GlobalType>::format(
    const ::wasp::binary::GlobalType& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{} {}", self.mut, self.valtype);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Import>::format(
    const ::wasp::binary::Import& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{{module \"{}\", name \"{}\", desc {}", self.module,
            self.name, self.kind());

  switch (self.kind()) {
    case ::wasp::binary::ExternalKind::Function:
      format_to(buf, " {}}}", self.index());
      break;

    case ::wasp::binary::ExternalKind::Table:
      format_to(buf, " {}}}", self.table_type());
      break;

    case ::wasp::binary::ExternalKind::Memory:
      format_to(buf, " {}}}", self.memory_type());
      break;

    case ::wasp::binary::ExternalKind::Global:
      format_to(buf, " {}}}", self.global_type());
      break;

    default:
      break;
  }

  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Export>::format(
    const ::wasp::binary::Export& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{{name \"{}\", desc {} {}}}", self.name, self.kind,
            self.index);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Expression>::format(
    const ::wasp::binary::Expression& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{}", self.data);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::ConstantExpression>::format(
    const ::wasp::binary::ConstantExpression& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{}", self.data);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Opcode>::format(
    const ::wasp::binary::Opcode& self,
    Ctx& ctx) {
  string_view result;
  switch (self) {
#define WASP_V(prefix, val, Name, str, ...) \
  case ::wasp::binary::Opcode::Name:        \
    result = str;                           \
    break;
#define WASP_FEATURE_V(...) WASP_V(__VA_ARGS__)
#define WASP_PREFIX_V(...) WASP_V(__VA_ARGS__)
#include "wasp/binary/opcode.def"
#undef WASP_V
#undef WASP_FEATURE_V
#undef WASP_PREFIX_V
    default: {
      // Special case for opcodes with unknown ids.
      memory_buffer buf;
      format_to(buf, "<unknown:{}>", static_cast<::wasp::u32>(self));
      return formatter<string_view>::format(to_string_view(buf), ctx);
    }
  }
  return formatter<string_view>::format(result, ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::CallIndirectImmediate>::format(
    const ::wasp::binary::CallIndirectImmediate& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{} {}", self.index, self.reserved);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::BrTableImmediate>::format(
    const ::wasp::binary::BrTableImmediate& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{} {}", self.targets, self.default_target);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::InitImmediate>::format(
    const ::wasp::binary::InitImmediate& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{} {}", self.segment_index, self.reserved);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Instruction>::format(
    const ::wasp::binary::Instruction& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{}", self.opcode);

  if (self.has_empty_immediate()) {
    // Nothing.
  } else if (self.has_block_type_immediate()) {
    format_to(buf, " {}", self.block_type_immediate());
  } else if (self.has_index_immediate()) {
    format_to(buf, " {}", self.index_immediate());
  } else if (self.has_call_indirect_immediate()) {
    format_to(buf, " {}", self.call_indirect_immediate());
  } else if (self.has_br_table_immediate()) {
    format_to(buf, " {}", self.br_table_immediate());
  } else if (self.has_u8_immediate()) {
    format_to(buf, " {}", self.u8_immediate());
  } else if (self.has_mem_arg_immediate()) {
    format_to(buf, " {}", self.mem_arg_immediate());
  } else if (self.has_s32_immediate()) {
    format_to(buf, " {}", self.s32_immediate());
  } else if (self.has_s64_immediate()) {
    format_to(buf, " {}", self.s64_immediate());
  } else if (self.has_f32_immediate()) {
    format_to(buf, " {:f}", self.f32_immediate());
  } else if (self.has_f64_immediate()) {
    format_to(buf, " {:f}", self.f64_immediate());
  } else if (self.has_init_immediate()) {
    format_to(buf, " {}", self.init_immediate());
  }
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Function>::format(
    const ::wasp::binary::Function& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{{type {}}}", self.type_index);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Table>::format(
    const ::wasp::binary::Table& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{{type {}}}", self.table_type);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Memory>::format(
    const ::wasp::binary::Memory& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{{type {}}}", self.memory_type);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Global>::format(
    const ::wasp::binary::Global& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{{type {}, init {}}}", self.global_type, self.init);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Start>::format(
    const ::wasp::binary::Start& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{{func {}}}", self.func_index);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::ElementSegment>::format(
    const ::wasp::binary::ElementSegment& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{{table {}, offset {}, init {}}}", self.table_index,
            self.offset, self.init);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::Code>::format(
    const ::wasp::binary::Code& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{{locals {}, body {}}}", self.locals, self.body);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::DataSegment>::format(
    const ::wasp::binary::DataSegment& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{{memory {}, offset {}, init {}}}", self.memory_index,
            self.offset, self.init);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::NameAssoc>::format(
    const ::wasp::binary::NameAssoc& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{} \"{}\"", self.index, self.name);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::IndirectNameAssoc>::format(
    const ::wasp::binary::IndirectNameAssoc& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{} {}", self.index, self.name_map);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

template <typename Ctx>
typename Ctx::iterator formatter<::wasp::binary::NameSubsection>::format(
    const ::wasp::binary::NameSubsection& self,
    Ctx& ctx) {
  memory_buffer buf;
  format_to(buf, "{} {}", self.id, self.data);
  return formatter<string_view>::format(to_string_view(buf), ctx);
}

}  // namespace fmt