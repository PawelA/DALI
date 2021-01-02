// Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
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

#include "dali/core/static_switch.h"
#include "dali/operators/image/mask/grid_mask.h"
#include "dali/kernels/mask/grid_mask_cpu.h"
#include "dali/core/format.h"

#define TYPES (uint8_t, int16_t, int32_t, float)

namespace dali {

DALI_SCHEMA(GridMask)
    .DocStr("doc")
    .NumInput(1)
    .NumOutput(1)
    .AddOptionalArg("tile", "doc", 100, true)
    .AddOptionalArg("ratio", "doc", 0.5f, true)
    .AddOptionalArg("angle", "doc", 0.0f, true);

bool GridMaskCpu::SetupImpl(std::vector<OutputDesc> &output_desc,
                            const workspace_t<CPUBackend> &ws) {
  const auto &input = ws.template InputRef<CPUBackend>(0);
  const auto &output = ws.template OutputRef<CPUBackend>(0);
  output_desc.resize(1);
  output_desc[0] = {input.shape(), input.type()};
  kernel_manager_.Resize(num_threads_, max_batch_size_);
  TYPE_SWITCH(input.type().id(), type2id, Type, TYPES, (
      {
          using Kernel = kernels::GridMaskCpu<Type>;
          kernel_manager_.Initialize<Kernel>();
      }
  ), DALI_FAIL(make_string("Unsupported input type: ", input.type().id()))) // NOLINT
  return true;
}


void GridMaskCpu::RunImpl(workspace_t<CPUBackend> &ws) {
  const auto &input = ws.template InputRef<CPUBackend>(0);
  auto &output = ws.template OutputRef<CPUBackend>(0);
  output.SetLayout(input.GetLayout());
  auto out_shape = output.shape();
  auto& tp = ws.GetThreadPool();
  TYPE_SWITCH(input.type().id(), type2id, Type, TYPES, (
      {
          using Kernel = kernels::GridMaskCpu<Type>;
          for (int sid = 0; sid < input.shape().num_samples(); sid++) {
            int tile = spec_.GetArgument<int>("tile", &ws, sid);
            float ratio = spec_.GetArgument<float>("ratio", &ws, sid);
            float angle = spec_.GetArgument<float>("angle", &ws, sid);
            tp.AddWork([&, sid](int tid) {
              kernels::KernelContext ctx;
              auto tvin = view<const Type>(input[sid]);
              auto tvout = view<Type>(output[sid]);
              kernel_manager_.Run<Kernel>(tid, sid, ctx, tvout, tvin,
                tile, ratio, angle);
            }, out_shape.tensor_size(sid));
          }
      }
  ), DALI_FAIL(make_string("Unsupported input type: ", input.type().id()))) // NOLINT
  tp.RunAll();
}

DALI_REGISTER_OPERATOR(GridMask, GridMaskCpu, CPU);

}  // namespace dali
