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

#pragma once

#include <memory>
#include "paddle/pten/backends/xpu/forwards.h"
#include "paddle/pten/common/place.h"
#include "paddle/pten/core/device_context.h"

#include "paddle/pten/backends/xpu/xpu_header.h"
#include "paddle/pten/backends/xpu/xpu_info.h"

namespace xpu = baidu::xpu::api;

namespace pten {

class XPUContext : public DeviceContext {
 public:
  XPUContext();

  explicit XPUContext(const XPUPlace&);

  virtual ~XPUContext();

  const Place& GetPlace() const override;

  backends::xpu::XPUVersion xpu_version() const;

  xpu::Context* x_context() const;

  // Return bkcl context.
  xpu::BKCLContext_t bkcl_context() const;
  void SetBkclContext(xpu::BKCLContext_t context);

  // Wait for all operations completion in the stream.
  void Wait() const override;

 public:
  // NOTE: DeviceContext hold resources. Used in training scenarios.
  // The interface used by the training scene, DeviceContext will initialize
  // all resources and delete them when destructing.
  void Init();

 public:
  // NOTE: External users manage resources. Used in inference scenarios.
  // The Set interface is for inference only, DeviceContext will mark the
  // resource as external, and will not delete any resource when destructing.
  void SetXContext(xpu::Context*);

  void SetL3Cache(int l3_size = 14155776);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace pten
