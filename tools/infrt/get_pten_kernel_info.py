#!/bin/python

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

import argparse
import json
import yaml


def parse_args():
    parser = argparse.ArgumentParser("gather pten kernel and infermate info")
    parser.add_argument(
        "--paddle_root_path",
        type=str,
        required=True,
        help="root path of paddle src[WORK_PATH/Paddle] .")
    parser.add_argument(
        "--kernel_info_file",
        type=str,
        required=True,
        help="kernel info file generated by get_pten_kernel_function.sh .")
    parser.add_argument(
        "--infermeta_wrap_file",
        type=str,
        required=True,
        help="inferMeta wrap info file .")
    args = parser.parse_args()
    return args


def get_api_yaml_info(file_path):
    f = open(file_path + "/python/paddle/utils/code_gen/api.yaml", "r")
    cont = f.read()
    return yaml.load(cont, Loader=yaml.FullLoader)


def get_kernel_info(file_path):
    f = open(file_path, "r")
    cont = f.readlines()
    return [l.strip() for l in cont]


def merge(infer_meta_data, kernel_data, wrap_data):
    meta_map = {}
    for api in infer_meta_data:
        if "kernel" not in api or "infer_meta" not in api:
            continue
        meta_map[api["kernel"]["func"]] = api["infer_meta"]["func"]
    wrap_map = {}
    for l in wrap_data:
        wrap_map[l.split()[0]] = l.split()[1]

    full_kernel_data = []
    for l in kernel_data:
        key = l.split()[0]
        if key in meta_map:
            if key in meta_map:
                full_kernel_data.append((l + " " + wrap_map[key]).split())
            else:
                full_kernel_data.append((l + " " + meta_map[key]).split())
        else:
            full_kernel_data.append((l + " unknown").split())

    return full_kernel_data


if __name__ == "__main__":
    args = parse_args()
    infer_meta_data = get_api_yaml_info(args.paddle_root_path)
    kernel_data = get_kernel_info(args.kernel_info_file)
    info_meta_wrap_data = get_kernel_info(args.infermeta_wrap_file)
    out = merge(infer_meta_data, kernel_data, info_meta_wrap_data)
    print(json.dumps(out))
