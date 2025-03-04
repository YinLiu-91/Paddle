/* Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "paddle/pten/kernels/full_kernel.h"

#include "paddle/pten/backends/gpu/gpu_context.h"
#include "paddle/pten/core/kernel_registry.h"
#include "paddle/pten/kernels/funcs/elementwise_base.h"
namespace pten {

template <typename InT, typename OutT = InT>
struct FullFuctor {
  OutT value;

  template <typename VType>
  explicit inline FullFuctor(VType val) {
    value = static_cast<OutT>(val);
  }

  __device__ __forceinline__ OutT operator()() const {
    return static_cast<OutT>(value);
  }
};

template <typename T, typename ContextT>
void FullKernel(const ContextT& dev_ctx,
                const ScalarArray& shape,
                const Scalar& val,
                DenseTensor* out) {
  out->Resize(paddle::framework::make_ddim(shape.GetData()));
  int numel = out->numel();
  out->mutable_data<T>(dev_ctx.GetPlace());
  if (numel > 0) {
    // in transformer model the numel of outpout will be zero.
    std::vector<const DenseTensor*> inputs = {};
    std::vector<DenseTensor*> outputs = {out};
    // This function has no input, so the inputs.size() == 0. Use kUnary, but
    // the data will not be loaded in the kernel because the number of
    // parameters in the operator is 0
    pten::funcs::LaunchSameDimsElementwiseCudaKernel<T>(
        dev_ctx, inputs, &outputs, FullFuctor<T>(val.to<T>()));
  }
}

template <typename T, typename ContextT>
void FullLikeKernel(const ContextT& dev_ctx,
                    const Scalar& val,
                    DenseTensor* out) {
  auto value = val.to<float>();
  using CommonType = typename std::common_type<
      float,
      typename std::conditional<std::is_same<T, pten::dtype::float16>::value,
                                float,
                                T>::type>::type;

  auto common_type_value = static_cast<CommonType>(value);

  PADDLE_ENFORCE_EQ(
      (common_type_value >=
       static_cast<CommonType>(std::numeric_limits<T>::lowest())) &&
          (common_type_value <=
           static_cast<CommonType>(std::numeric_limits<T>::max())),
      true,
      pten::errors::InvalidArgument(
          "The filled value is out of range for target type, "
          "current kernel type is %s, the range should between %f "
          "and %f, but now value is %f.",
          typeid(T).name(),
          static_cast<CommonType>(std::numeric_limits<T>::lowest()),
          static_cast<CommonType>(std::numeric_limits<T>::max()),
          static_cast<float>(value)));
  std::vector<const DenseTensor*> inputs = {};
  std::vector<DenseTensor*> outputs = {out};
  out->mutable_data<T>(dev_ctx.GetPlace());
  // This function has no input, so the inputs.size() == 0. Use kUnary, but the
  // data will not be loaded in the kernel because the number of parameters in
  // the operator is 0
  int numel = out->numel();
  if (numel > 0) {
    pten::funcs::LaunchSameDimsElementwiseCudaKernel<T>(
        dev_ctx, inputs, &outputs, FullFuctor<T>(value));
  }
}

}  // namespace pten

PT_REGISTER_KERNEL(full,
                   GPU,
                   ALL_LAYOUT,
                   pten::FullKernel,
                   float,
                   double,
                   uint8_t,
                   int16_t,
                   int,
                   int64_t,
                   bool,
                   paddle::platform::float16,
                   paddle::platform::complex<float>,
                   paddle::platform::complex<double>) {}

PT_REGISTER_KERNEL(full_like,
                   GPU,
                   ALL_LAYOUT,
                   pten::FullLikeKernel,
                   float,
                   double,
                   int,
                   int64_t,
                   bool,
                   paddle::platform::float16) {}
