/* Copyright (c) 2022 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#pragma once

#include "paddle/pten/core/dense_tensor.h"
#include "paddle/pten/infermeta/unary.h"
#include "paddle/pten/kernels/empty_kernel.h"

namespace pten {

template <typename Context>
void TransferLayoutKernel(const Context& dev_ctx,
                          const DenseTensor& x,
                          DataLayout dst_layout,
                          DenseTensor* out);

template <typename Context>
DenseTensor TransferLayout(const Context& dev_ctx,
                           const DenseTensor& x,
                           DataLayout dst_layout) {
  pten::DenseTensor dense_out =
      pten::Empty(dev_ctx, {x.dtype(), x.dims(), dst_layout});
  TransferLayoutKernel<Context>(dev_ctx, x, dst_layout, &dense_out);
  return dense_out;
}

}  // namespace pten
