// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
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

#pragma once

// See Note [ Why still include the fluid headers? ]
#include "paddle/fluid/platform/for_range.h"
#include "paddle/pten/kernels/funcs/complex_functors.h"

namespace pten {

template <typename T, typename Context>
void ConjKernel(const Context& dev_ctx,
                const DenseTensor& x,
                DenseTensor* out) {
  auto numel = x.numel();
  auto* x_data = x.data<T>();
  auto* out_data = dev_ctx.template Alloc<T>(out);

  paddle::platform::ForRange<Context> for_range(dev_ctx, numel);
  pten::funcs::ConjFunctor<T> functor(x_data, numel, out_data);
  for_range(functor);
}

}  // namespace pten
