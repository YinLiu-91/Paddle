//   Copyright (c) 2022 PaddlePaddle Authors. All Rights Reserved.
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

#include "paddle/pten/core/device_context.h"
#include "paddle/pten/core/enforce.h"
#include "paddle/pten/core/tensor_base.h"

namespace pten {
using DataType = paddle::experimental::DataType;

struct DeviceContext::Impl {
  Impl() = default;
  ~Impl() = default;

  void SetAllocator(const Allocator* allocator) {
    PADDLE_ENFORCE_NOT_NULL(
        allocator,
        pten::errors::InvalidArgument(
            "Required allocator shall not be nullptr, but received nullptr."));
    device_allocator_ = allocator;
  }

  void SetHostAllocator(const Allocator* allocator) {
    PADDLE_ENFORCE_NOT_NULL(
        allocator,
        pten::errors::InvalidArgument(
            "Required allocator shall not be nullptr, but received nullptr."));
    host_allocator_ = allocator;
  }

  void SetZeroAllocator(const Allocator* allocator) {
    PADDLE_ENFORCE_NOT_NULL(
        allocator,
        pten::errors::InvalidArgument(
            "Required allocator shall not be nullptr, but received nullptr."));
    zero_allocator_ = allocator;
  }

  const Allocator& GetAllocator() const {
    PADDLE_ENFORCE_NOT_NULL(
        device_allocator_,
        pten::errors::InvalidArgument("Required device_allocator_ shall not be "
                                      "nullptr, but received nullptr."));
    return *device_allocator_;
  }

  const Allocator& GetHostAllocator() const {
    PADDLE_ENFORCE_NOT_NULL(
        host_allocator_,
        pten::errors::InvalidArgument("Required host_allocator_ shall not be "
                                      "nullptr, but received nullptr."));
    return *host_allocator_;
  }

  const Allocator& GetZeroAllocator() const {
    PADDLE_ENFORCE_NOT_NULL(
        zero_allocator_,
        pten::errors::InvalidArgument("Required host_allocator_ shall not be "
                                      "nullptr, but received nullptr."));
    return *zero_allocator_;
  }

  void* Alloc(TensorBase* tensor,
              DataType dtype = DataType::UNDEFINED,
              size_t requested_size = 0) const {
    PADDLE_ENFORCE_NOT_NULL(
        tensor,
        pten::errors::InvalidArgument(
            "Required tensor shall not be nullptr, but received nullptr."));
    if (dtype == DataType::UNDEFINED) {
      dtype = tensor->dtype();
    }
    auto* allocator =
        tensor->numel() == 0 ? zero_allocator_ : device_allocator_;
    return tensor->AllocateFrom(
        const_cast<Allocator*>(allocator), dtype, requested_size);
  }

  template <typename T>
  T* Alloc(TensorBase* tensor, size_t requested_size = 0) const {
    DataType dtype = paddle::experimental::CppTypeToDataType<T>::Type();
    return static_cast<T*>(Alloc(tensor, dtype, requested_size));
  }

  void* HostAlloc(TensorBase* tensor,
                  DataType dtype = DataType::UNDEFINED,
                  size_t requested_size = 0) const {
    PADDLE_ENFORCE_NOT_NULL(
        tensor,
        pten::errors::InvalidArgument(
            "Required tensor shall not be nullptr, but received nullptr."));
    if (dtype == DataType::UNDEFINED) {
      dtype = tensor->dtype();
    }
    auto* allocator = tensor->numel() == 0 ? zero_allocator_ : host_allocator_;
    return tensor->AllocateFrom(
        const_cast<Allocator*>(allocator), dtype, requested_size);
  }

  template <typename T>
  T* HostAlloc(pten::TensorBase* tensor, size_t requested_size = 0) const {
    DataType dtype = paddle::experimental::CppTypeToDataType<T>::Type();
    return static_cast<T*>(HostAlloc(tensor, dtype, requested_size));
  }

  void SetGenerator(Generator* gen) {
    PADDLE_ENFORCE_NOT_NULL(
        gen,
        pten::errors::InvalidArgument(
            "Required generator shall not be nullptr, but received nullptr."));
    generator_ = gen;
  }

  Generator* GetGenerator() const {
    PADDLE_ENFORCE_NOT_NULL(
        generator_,
        pten::errors::InvalidArgument("Required generator_ shall not be "
                                      "nullptr, but received nullptr."));
    return generator_;
  }

 private:
  const Allocator* device_allocator_{nullptr};
  const Allocator* host_allocator_{nullptr};
  const Allocator* zero_allocator_{nullptr};
  Generator* generator_{nullptr};
};

DeviceContext::DeviceContext() { impl_ = std::make_unique<Impl>(); }

DeviceContext::DeviceContext(const DeviceContext& other) {
  impl_->SetHostAllocator(&other.GetHostAllocator());
  impl_->SetAllocator(&other.GetAllocator());
  impl_->SetZeroAllocator(&other.GetZeroAllocator());
}

DeviceContext::DeviceContext(DeviceContext&& other) {
  impl_ = std::move(other.impl_);
}

DeviceContext::~DeviceContext() = default;

void DeviceContext::SetAllocator(const Allocator* allocator) {
  impl_->SetAllocator(allocator);
}

const Allocator& DeviceContext::GetAllocator() const {
  return impl_->GetAllocator();
}

void DeviceContext::SetHostAllocator(const Allocator* allocator) {
  impl_->SetHostAllocator(allocator);
}

const Allocator& DeviceContext::GetHostAllocator() const {
  return impl_->GetHostAllocator();
}

void DeviceContext::SetZeroAllocator(const Allocator* allocator) {
  impl_->SetZeroAllocator(allocator);
}

const Allocator& DeviceContext::GetZeroAllocator() const {
  return impl_->GetZeroAllocator();
}

void* DeviceContext::Alloc(TensorBase* tensor,
                           DataType dtype,
                           size_t requested_size) const {
  return impl_->Alloc(tensor, dtype, requested_size);
}

template <typename T>
T* DeviceContext::Alloc(TensorBase* tensor, size_t requested_size) const {
  return impl_->Alloc<T>(tensor, requested_size);
}

void* DeviceContext::HostAlloc(TensorBase* tensor,
                               DataType dtype,
                               size_t requested_size) const {
  return impl_->HostAlloc(tensor, dtype, requested_size);
}

template <typename T>
T* DeviceContext::HostAlloc(TensorBase* tensor, size_t requested_size) const {
  return impl_->HostAlloc<T>(tensor, requested_size);
}

#define DEVICE_CONTEXT_MEMBER_FUNC_INSTANTIATION(dtype)              \
  template dtype* DeviceContext::Alloc(TensorBase* tensor,           \
                                       size_t requested_size) const; \
  template dtype* DeviceContext::HostAlloc(TensorBase* tensor,       \
                                           size_t requested_size) const;

DEVICE_CONTEXT_MEMBER_FUNC_INSTANTIATION(bool)
DEVICE_CONTEXT_MEMBER_FUNC_INSTANTIATION(int8_t)
DEVICE_CONTEXT_MEMBER_FUNC_INSTANTIATION(uint8_t)
DEVICE_CONTEXT_MEMBER_FUNC_INSTANTIATION(int16_t)
DEVICE_CONTEXT_MEMBER_FUNC_INSTANTIATION(int32_t)
DEVICE_CONTEXT_MEMBER_FUNC_INSTANTIATION(int64_t)
DEVICE_CONTEXT_MEMBER_FUNC_INSTANTIATION(float)
DEVICE_CONTEXT_MEMBER_FUNC_INSTANTIATION(double)
DEVICE_CONTEXT_MEMBER_FUNC_INSTANTIATION(::paddle::experimental::bfloat16)
DEVICE_CONTEXT_MEMBER_FUNC_INSTANTIATION(::paddle::experimental::float16)
DEVICE_CONTEXT_MEMBER_FUNC_INSTANTIATION(::paddle::experimental::complex64)
DEVICE_CONTEXT_MEMBER_FUNC_INSTANTIATION(::paddle::experimental::complex128)

#undef DEVICE_CONTEXT_MEMBER_FUNC_INSTANTIATION

void DeviceContext::SetGenerator(Generator* gen) { impl_->SetGenerator(gen); }

Generator* DeviceContext::GetGenerator() const { return impl_->GetGenerator(); }

}  // namespace pten
