/*******************************************************************************
* Copyright 2017-2018 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/* Copyright 2016 The TensorFlow Authors. All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#pragma once

#include <vector>
#include "ngraph/node.hpp"
#include "ngraph/op/constant.hpp"
#include "ngraph/runtime/cpu/op/quantize.hpp"
#include "ngraph/util.hpp"

namespace ngraph
{
    namespace runtime
    {
        namespace cpu
        {
            namespace quantization_util
            {
                static inline void get_min_max_range(const ngraph::Node* node,
                                                     std::vector<float>& quant_util)
                {
                    auto quantize = static_cast<const ngraph::op::Quantize*>(node);
                    auto min_const_op =
                        std::dynamic_pointer_cast<ngraph::op::Constant>(quantize->get_argument(1));
                    auto max_const_op =
                        std::dynamic_pointer_cast<ngraph::op::Constant>(quantize->get_argument(2));
                    float input_min_range =
                        *(static_cast<float const*>(min_const_op->get_data_ptr()));
                    float input_max_range =
                        *(static_cast<float const*>(max_const_op->get_data_ptr()));
                    // begin code copied and pasted (and modified) from mkl_quantize_op.cc
                    float min_range;
                    float max_range;
                    // If input_min_range and input_max_range are close,
                    // introduce a slightly larger delta between them.
                    min_range = std::min(0.0f, input_min_range);
                    const float epsilon =
                        std::max(1.0f, std::max(fabsf(input_min_range), fabsf(input_max_range))) /
                        100.0f;
                    max_range = std::max(input_max_range, min_range + epsilon);
                    max_range = std::max(0.0f, max_range);
                    const float max_abs = std::max(std::abs(min_range), std::abs(max_range));
                    bool is_signed = (quantize->get_quantize_et()).is_signed();
                    // ...end code copied and pasted (and modified) from mkl_quantize_op.cc
                    const float target_range = (is_signed ? std::pow(2, 7) : std::pow(2, 8)) - 1;
                    max_range = max_abs;
                    min_range = is_signed ? -max_abs : 0;
                    const float scale_factor = target_range / max_abs;
                    quant_util.push_back(min_range);
                    quant_util.push_back(max_range);
                    quant_util.push_back(scale_factor);
                }
            }
        }
    }
}
