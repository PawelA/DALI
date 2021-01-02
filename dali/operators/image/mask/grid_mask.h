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

#ifndef DALI_OPERATORS_IMAGE_MASK_GRID_MASK_H_
#define DALI_OPERATORS_IMAGE_MASK_GRID_MASK_H_

#include <vector>
#include "dali/kernels/kernel_manager.h"
#include "dali/pipeline/operator/common.h"
#include "dali/pipeline/operator/operator.h"

namespace dali {

template <typename Backend>
class GridMask : public Operator<Backend> {
 public:
  explicit GridMask(const OpSpec &spec) : Operator<Backend>(spec) { }

 protected:
  bool CanInferOutputs() const override { return true; }
  kernels::KernelManager kernel_manager_;
};


class GridMaskCpu : public GridMask<CPUBackend> {
 public:
  explicit GridMaskCpu(const OpSpec &spec) : GridMask(spec) { }

 protected:
  bool SetupImpl(std::vector<OutputDesc> &output_desc, const workspace_t<CPUBackend> &ws) override;
  void RunImpl(workspace_t<CPUBackend> &ws) override;
};

}  // namespace dali

#endif  // DALI_OPERATORS_IMAGE_MASK_GRID_MASK_H_
