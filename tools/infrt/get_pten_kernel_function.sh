#!/usr/bin/env bash

# Copyright (c) 2022 PaddlePaddle Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#=================================================
#                   Utils
#=================================================

set -e

#step 1:get kernel registered info
kernel_register_info_file=`mktemp`
PADDLE_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}")/../../" && pwd )"
unset GREP_OPTIONS && find ${PADDLE_ROOT}/paddle/pten/kernels -name "*.c*" \
   | xargs sed -e '/PT_REGISTER_\(GENERAL_\)\?KERNEL(/,/)/!d' \
   | awk 'BEGIN { RS="{" }{ gsub(/\n /,""); print $0 }' \
   | grep PT_REGISTER \
   | awk -F ",|\(" '{gsub(/ /,"");print $2, $3, $4, $5}' \
   |  sort -u | awk '{gsub(/pten::/,"");print $0}' \
   | grep -v "_grad" > $kernel_register_info_file

#step 2:get simple general inferMeta function wrap info
temp_path=`mktemp -d`
python3 ${PADDLE_ROOT}/python/paddle/utils/code_gen/wrapped_infermeta_gen.py \
  --api_yaml_path ${PADDLE_ROOT}/python/paddle/utils/code_gen/api.yaml \
  --wrapped_infermeta_header_path ${temp_path}/generate.h \
  --wrapped_infermeta_source_path ${temp_path}/generate.cc

grep PT_REGISTER_INFER_META_FN ${temp_path}/generate.cc  \
  | awk -F "\(|,|::|\)" '{print $2, $4}' > ${temp_path}/wrap_info.txt

#step 3: merge all infos
#  @input1 => pten kernel infomation : kernel_name kernel_key(GPU/CPU, precision, layout)
#  @input2 => information from api.yaml : kernel_name kernel_function_name inferMeta_function_name 
#  @input3 => information from wrapped_infermeta_gen : ensure the inferMeta function has
#             same signature with kernel function
python3 ${PADDLE_ROOT}/tools/infrt/get_pten_kernel_info.py \
  --paddle_root_path ${PADDLE_ROOT} \
  --kernel_info_file $kernel_register_info_file \
  --infermeta_wrap_file ${temp_path}/wrap_info.txt
