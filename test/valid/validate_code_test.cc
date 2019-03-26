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

#include "gtest/gtest.h"

#include "wasp/base/features.h"
#include "wasp/binary/formatters.h"
#include "wasp/valid/begin_code.h"
#include "wasp/valid/context.h"
#include "wasp/valid/test_utils.h"
#include "wasp/valid/validate_instruction.h"
#include "wasp/valid/validate_locals.h"

using namespace ::wasp;
using namespace ::wasp::binary;
using namespace ::wasp::valid;
using namespace ::wasp::valid::test;

TEST(ValidateCodeTest, BeginCode) {
  Context context;
  context.types.push_back(TypeEntry{FunctionType{}});
  context.functions.push_back(Function{0});
  TestErrors errors;
  EXPECT_TRUE(BeginCode(context, Features{}, errors));
}

TEST(ValidateCodeTest, BeginCode_CodeIndexOOB) {
  Context context;
  context.types.push_back(TypeEntry{FunctionType{}});
  context.functions.push_back(Function{0});
  context.code_count = 1;
  TestErrors errors;
  EXPECT_FALSE(BeginCode(context, Features{}, errors));
}

TEST(ValidateCodeTest, BeginCode_TypeIndexOOB) {
  Context context;
  context.types.push_back(TypeEntry{FunctionType{}});
  context.functions.push_back(Function{1});
  TestErrors errors;
  EXPECT_FALSE(BeginCode(context, Features{}, errors));
}

TEST(ValidateCodeTest, Locals) {
  Context context;
  TestErrors errors;
  EXPECT_TRUE(
      Validate(Locals{10, ValueType::I32}, context, Features{}, errors));
}

class ValidateInstructionTest : public ::testing::Test {
 protected:
  using I = Instruction;
  using O = Opcode;

  virtual void SetUp() {
    BeginFunction(FunctionType{});
  }

  virtual void TearDown() {}

  void BeginFunction(const FunctionType& function_type) {
    context = Context{};
    AddFunction(function_type);
    EXPECT_TRUE(BeginCode(context, features, errors));
  }

  Index AddFunctionType(const FunctionType& function_type) {
    context.types.push_back(TypeEntry{function_type});
    return context.types.size() - 1;
  }

  Index AddFunction(const FunctionType& function_type) {
    Index type_index = AddFunctionType(function_type);
    context.functions.push_back(Function{type_index});
    return context.functions.size() - 1;
  }

  Index AddTable(const TableType& table_type) {
    context.tables.push_back(table_type);
    return context.tables.size() - 1;
  }

  void Step(const Instruction& instruction) {
    EXPECT_TRUE(Validate(instruction, context, features, errors))
        << format("{}", instruction);
  }

  void Fail(const Instruction& instruction) {
    EXPECT_FALSE(Validate(instruction, context, features, errors))
        << format("{}", instruction);
  }

  Context context;
  Features features;
  TestErrors errors;
};

struct ValueTypeInfo {
  ValueType value_type;
  BlockType block_type;
  Instruction instruction;
};
const ValueTypeInfo all_value_types[] = {
    {ValueType::I32, BlockType::I32, Instruction{Opcode::I32Const, s32{}}},
    {ValueType::I64, BlockType::I64, Instruction{Opcode::I64Const, s64{}}},
    {ValueType::F32, BlockType::F32, Instruction{Opcode::F32Const, f32{}}},
    {ValueType::F64, BlockType::F64, Instruction{Opcode::F64Const, f64{}}},
#if 0
    {ValueType::V128, BlockType::V128, Instruction{Opcode::V128Const, v128{}}},
    {ValueType::Anyref, BlockType::Anyref, Instruction{Opcode::RefNull}},
#endif
};

TEST_F(ValidateInstructionTest, Unreachable) {
  Step(I{O::Unreachable});
}

TEST_F(ValidateInstructionTest, Nop) {
  Step(I{O::Nop});
}

TEST_F(ValidateInstructionTest, Block_Void) {
  Step(I{O::Block, BlockType::Void});
  Step(I{O::End});
}

TEST_F(ValidateInstructionTest, Block_SingleResult) {
  for (const auto& info : all_value_types) {
    Step(I{O::Block, info.block_type});
    Step(info.instruction);
    Step(I{O::End});
  }
}

TEST_F(ValidateInstructionTest, Loop_Void) {
  Step(I{O::Loop, BlockType::Void});
  Step(I{O::End});
}

TEST_F(ValidateInstructionTest, Loop_SingleResult) {
  for (const auto& info : all_value_types) {
    Step(I{O::Loop, info.block_type});
    Step(info.instruction);
    Step(I{O::End});
  }
}

TEST_F(ValidateInstructionTest, End) {
  Step(I{O::Block, BlockType::Void});
  Step(I{O::End});
}

TEST_F(ValidateInstructionTest, End_Unreachable) {
  Step(I{O::Block, BlockType::Void});
  Step(I{O::Unreachable});
  Step(I{O::End});

  Step(I{O::Block, BlockType::I32});
  Step(I{O::Unreachable});
  Step(I{O::End});

  Step(I{O::Block, BlockType::I32});
  Step(I{O::Unreachable});
  Step(I{O::I32Const, s32{}});
  Step(I{O::End});
}

TEST_F(ValidateInstructionTest, End_Unreachable_TypeMismatch) {
  Step(I{O::Block, BlockType::I32});
  Step(I{O::Unreachable});
  Step(I{O::F32Const, f32{}});
  Fail(I{O::End});
}

TEST_F(ValidateInstructionTest, End_EmptyLabelStack) {
  Step(I{O::End});  // This `end` ends the function.
  Fail(I{O::End});
}

TEST_F(ValidateInstructionTest, End_EmptyTypeStack) {
  Step(I{O::Block, BlockType::I32});
  Fail(I{O::End});
}

TEST_F(ValidateInstructionTest, End_TypeMismatch) {
  Step(I{O::Block, BlockType::I32});
  Step(I{O::F32Const, f32{}});
  Fail(I{O::End});
}

TEST_F(ValidateInstructionTest, End_TooManyValues) {
  Step(I{O::Block, BlockType::I32});
  Step(I{O::I32Const, s32{}});
  Step(I{O::I32Const, s32{}});
  Fail(I{O::End});
}

TEST_F(ValidateInstructionTest, End_Unreachable_TooManyValues) {
  Step(I{O::Block, BlockType::I32});
  Step(I{O::Unreachable});
  Step(I{O::I32Const, s32{}});
  Step(I{O::I32Const, s32{}});
  Fail(I{O::End});
}

TEST_F(ValidateInstructionTest, Br_Void) {
  Step(I{O::Br, Index{0}});
}

TEST_F(ValidateInstructionTest, Br_Block_SingleResult) {
  for (const auto& info : all_value_types) {
    Step(I{O::Block, info.block_type});
    Step(info.instruction);
    Step(I{O::Br, Index{0}});
    Step(I{O::End});
  }
}

TEST_F(ValidateInstructionTest, Br_EmptyStack) {
  Step(I{O::Block, BlockType::I32});
  Fail(I{O::Br, Index{0}});
}

TEST_F(ValidateInstructionTest, Br_FullerStack) {
  Step(I{O::Block, BlockType::Void});
  Step(I{O::I32Const, s32{}});
  Step(I{O::Br, Index{0}});
}

TEST_F(ValidateInstructionTest, Br_TypeMismatch) {
  Step(I{O::Block, BlockType::I32});
  Step(I{O::F32Const, f32{}});
  Fail(I{O::Br, Index{0}});
}

TEST_F(ValidateInstructionTest, Br_Depth1) {
  Step(I{O::Block, BlockType::I32});
  Step(I{O::Block, BlockType::Void});
  Step(I{O::I32Const, s32{}});
  Step(I{O::Br, Index{1}});
}

TEST_F(ValidateInstructionTest, Br_ForwardUnreachable) {
  Step(I{O::Block, BlockType::I32});
  Step(I{O::Block, BlockType::F32});
  Step(I{O::I32Const, s32{}});
  Step(I{O::Br, Index{1}});
  Step(I{O::Br, Index{0}});
}

TEST_F(ValidateInstructionTest, Br_Loop_Void) {
  Step(I{O::Loop, BlockType::Void});
  Step(I{O::Br, Index{0}});
  Step(I{O::End});

  Step(I{O::Loop, BlockType::I32});
  Step(I{O::Br, Index{0}});
  Step(I{O::End});
}

TEST_F(ValidateInstructionTest, Br_Loop_SingleResult) {
  for (const auto& info : all_value_types) {
    Step(I{O::Loop, info.block_type});
    Step(I{O::Br, Index{0}});
    Step(I{O::End});
  }
}

TEST_F(ValidateInstructionTest, BrIf_Void) {
  Step(I{O::I32Const, s32{}});
  Step(I{O::BrIf, Index{0}});
}

TEST_F(ValidateInstructionTest, BrIf_Block_SingleResult) {
  for (const auto& info : all_value_types) {
    Step(I{O::Block, info.block_type});
    Step(info.instruction);
    Step(I{O::I32Const, s32{}});
    Step(I{O::BrIf, Index{0}});
    Step(I{O::End});
  }
}

TEST_F(ValidateInstructionTest, BrIf_NoCondition) {
  Fail(I{O::BrIf, Index{0}});
}

TEST_F(ValidateInstructionTest, BrIf_ConditionMismatch) {
  Step(I{O::F32Const, f32{}});
  Fail(I{O::BrIf, Index{0}});
}

TEST_F(ValidateInstructionTest, BrIf_EmptyStack) {
  Step(I{O::Block, BlockType::I32});
  Step(I{O::I32Const, s32{}});
  Fail(I{O::BrIf, Index{0}});
}

TEST_F(ValidateInstructionTest, BrIf_TypeMismatch) {
  Step(I{O::Block, BlockType::I32});
  Step(I{O::F32Const, f32{}});
  Step(I{O::I32Const, s32{}});
  Fail(I{O::BrIf, Index{0}});
}

TEST_F(ValidateInstructionTest, BrIf_PropagateValue) {
  Step(I{O::Block, BlockType::F32});
  Step(I{O::Block, BlockType::I32});
  Step(I{O::F32Const, f32{}});
  Step(I{O::I32Const, s32{}});
  Step(I{O::BrIf, Index{1}});
  Fail(I{O::End});  // F32 is still on the stack.
}

TEST_F(ValidateInstructionTest, BrIf_Loop_Void) {
  Step(I{O::Loop, BlockType::Void});
  Step(I{O::I32Const, s32{}});
  Step(I{O::BrIf, Index{0}});
  Step(I{O::End});
}

TEST_F(ValidateInstructionTest, BrIf_Loop_SingleResult) {
  for (const auto& info : all_value_types) {
    Step(I{O::Loop, info.block_type});
    Step(info.instruction);
    Step(I{O::I32Const, s32{}});
    Step(I{O::BrIf, Index{0}});
    Step(I{O::Unreachable});
    Step(I{O::End});
  }
}

TEST_F(ValidateInstructionTest, BrTable_Void) {
  Step(I{O::I32Const, s32{}});
  Step(I{O::BrTable, BrTableImmediate{{0, 0, 0}, 0}});
}

TEST_F(ValidateInstructionTest, BrTable_MultiDepth_Void) {
  Step(I{O::Block, BlockType::Void});  // 3
  Step(I{O::Block, BlockType::Void});  // 2
  Step(I{O::Block, BlockType::Void});  // 1
  Step(I{O::Block, BlockType::Void});  // 0
  Step(I{O::I32Const, s32{}});
  Step(I{O::BrTable, BrTableImmediate{{0, 1, 2, 3}, 4}});
}

TEST_F(ValidateInstructionTest, BrTable_MultiDepth_SingleResult) {
  Step(I{O::Block, BlockType::I32});   // 3
  Step(I{O::Block, BlockType::Void});  // 2
  Step(I{O::Block, BlockType::I32});   // 1
  Step(I{O::Block, BlockType::Void});  // 0
  Step(I{O::I32Const, s32{}});
  Step(I{O::I32Const, s32{}});
  Step(I{O::BrTable, BrTableImmediate{{1, 1, 1, 3}, 3}});
}

TEST_F(ValidateInstructionTest, BrTable_Unreachable) {
  Step(I{O::Block, BlockType::I32});
  Step(I{O::I32Const, s32{}});
  Step(I{O::BrTable, BrTableImmediate{{}, 1}});
  Step(I{O::End});
}

TEST_F(ValidateInstructionTest, BrTable_NoKey) {
  Fail(I{O::BrTable, BrTableImmediate{{}, 0}});
}

TEST_F(ValidateInstructionTest, BrTable_EmptyStack) {
  Step(I{O::Block, BlockType::I32});
  Step(I{O::I32Const, s32{}});
  Fail(I{O::BrTable, BrTableImmediate{{}, 0}});
}

TEST_F(ValidateInstructionTest, BrTable_ValueTypeMismatch) {
  Step(I{O::Block, BlockType::I32});
  Step(I{O::F32Const, f32{}});
  Step(I{O::I32Const, s32{}});
  Fail(I{O::BrTable, BrTableImmediate{{0}, 0}});
}

TEST_F(ValidateInstructionTest, BrTable_InconsistentLabelSignature) {
  Step(I{O::Block, BlockType::Void});
  Step(I{O::Block, BlockType::I32});
  Step(I{O::I32Const, s32{}});
  Step(I{O::I32Const, s32{}});
  Fail(I{O::BrTable, BrTableImmediate{{1}, 0}});
}

TEST_F(ValidateInstructionTest, Return) {
  Step(I{O::Return});
}

TEST_F(ValidateInstructionTest, Return_InsideBlocks) {
  Step(I{O::Block, BlockType::Void});
  Step(I{O::Block, BlockType::Void});
  Step(I{O::Block, BlockType::Void});
  Step(I{O::Return});
}

TEST_F(ValidateInstructionTest, Return_Unreachable) {
  Step(I{O::Block, BlockType::F64});
  Step(I{O::Return});
  Step(I{O::End});
}

TEST_F(ValidateInstructionTest, Return_SingleResult) {
  BeginFunction(FunctionType{{}, {ValueType::I32}});
  Step(I{O::I32Const, s32{}});
  Step(I{O::Return});
}

TEST_F(ValidateInstructionTest, Return_TypeMismatch) {
  BeginFunction(FunctionType{{}, {ValueType::I32}});
  Step(I{O::F32Const, f32{}});
  Fail(I{O::Return});
}

TEST_F(ValidateInstructionTest, Call_Void_Void) {
  auto index = AddFunction(FunctionType{});
  Step(I{O::Call, Index{index}});
}

TEST_F(ValidateInstructionTest, Call_Params) {
  auto index = AddFunction(FunctionType{{ValueType::I32, ValueType::F32}, {}});
  Step(I{O::I32Const, s32{}});
  Step(I{O::F32Const, f32{}});
  Step(I{O::Call, Index{index}});
  Step(I{O::End});
}

TEST_F(ValidateInstructionTest, Call_SingleResult) {
  auto index = AddFunction(FunctionType{{}, {ValueType::F64}});
  Step(I{O::Block, BlockType::F64});
  Step(I{O::Call, Index{index}});
  Step(I{O::End});
}

TEST_F(ValidateInstructionTest, Call_EmptyStack) {
  auto index = AddFunction(FunctionType{{ValueType::I32}, {}});
  Fail(I{O::Call, Index{index}});
}

TEST_F(ValidateInstructionTest, Call_TypeMismatch) {
  auto index = AddFunction(FunctionType{{ValueType::I32}, {}});
  Step(I{O::F32Const, f32{}});
  Fail(I{O::Call, Index{index}});
}

TEST_F(ValidateInstructionTest, Call_FunctionIndexOOB) {
  Fail(I{O::Call, Index{100}});
}

TEST_F(ValidateInstructionTest, Call_TypeIndexOOB) {
  context.functions.push_back(Function{100});
  Index index = context.functions.size() - 1;
  Fail(I{O::Call, Index{index}});
}

TEST_F(ValidateInstructionTest, CallIndirect) {
  AddTable(TableType{Limits{0}, ElementType::Funcref});
  auto index = AddFunctionType(FunctionType{});
  Step(I{O::I32Const, s32{}});
  Step(I{O::CallIndirect, CallIndirectImmediate{index, 0}});
}

TEST_F(ValidateInstructionTest, CallIndirect_Params) {
  AddTable(TableType{Limits{0}, ElementType::Funcref});
  auto index =
      AddFunctionType(FunctionType{{ValueType::F32, ValueType::I64}, {}});
  Step(I{O::F32Const, f32{}});
  Step(I{O::I64Const, s64{}});
  Step(I{O::I32Const, s32{}});
  Step(I{O::CallIndirect, CallIndirectImmediate{index, 0}});
  Step(I{O::End});
}

TEST_F(ValidateInstructionTest, CallIndirect_SingleResult) {
  AddTable(TableType{Limits{0}, ElementType::Funcref});
  auto index = AddFunctionType(FunctionType{{}, {ValueType::F64}});
  Step(I{O::Block, BlockType::F64});
  Step(I{O::I32Const, s32{}});
  Step(I{O::CallIndirect, CallIndirectImmediate{index, 0}});
  Step(I{O::End});
}

TEST_F(ValidateInstructionTest, CallIndirect_TableIndexOOB) {
  auto index = AddFunctionType(FunctionType{});
  Step(I{O::I32Const, s32{}});
  Fail(I{O::CallIndirect, CallIndirectImmediate{index, 0}});
}

TEST_F(ValidateInstructionTest, CallIndirect_EmptyStack) {
  AddTable(TableType{Limits{0}, ElementType::Funcref});
  auto index = AddFunctionType(FunctionType{});
  Fail(I{O::CallIndirect, CallIndirectImmediate{index, 0}});
}

TEST_F(ValidateInstructionTest, CallIndirect_KeyTypeMismatch) {
  AddTable(TableType{Limits{0}, ElementType::Funcref});
  auto index = AddFunctionType(FunctionType{});
  Step(I{O::F32Const, f32{}});
  Fail(I{O::CallIndirect, CallIndirectImmediate{index, 0}});
}

TEST_F(ValidateInstructionTest, CallIndirect_TypeIndexOOB) {
  AddTable(TableType{Limits{0}, ElementType::Funcref});
  Step(I{O::I32Const, s32{}});
  Fail(I{O::CallIndirect, CallIndirectImmediate{100, 0}});
}

TEST_F(ValidateInstructionTest, Drop) {
  for (const auto& info : all_value_types) {
    Step(info.instruction);
    Step(I{O::Drop});
  }
}

TEST_F(ValidateInstructionTest, Drop_EmptyStack) {
  Fail(I{O::Drop});
}
