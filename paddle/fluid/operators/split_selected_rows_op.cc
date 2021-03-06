/* Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "paddle/fluid/operators/split_selected_rows_op.h"

namespace paddle {
namespace operators {

class SplitSelectedRowsOpMaker : public framework::OpProtoAndCheckerMaker {
 public:
  void Make() override {
    AddInput("X", "The input SelectedRows.");
    AddOutput("Out", "The outputs of the input SelectedRows.").AsDuplicable();
    AddAttr<std::vector<int64_t>>("height_sections",
                                  "Height for each output SelectedRows.")
        .SetDefault(std::vector<int64_t>({}));

    AddComment(R"DOC(
Split a SelectedRows with a specified rows section.
height_sections is only needed when need to split the dims of the original tensor.

Example:
  Input:
    X.rows = {7, 5}
    X.height = 12
  Attr:
    height_sections = {4, 8}
  Out:
    out0.rows = {}
    out0.height = 4

    out1.rows = {5, 7}
    out2.height = 8

)DOC");
  }
};

class SplitSelectedRowsOp : public framework::OperatorWithKernel {
 public:
  using framework::OperatorWithKernel::OperatorWithKernel;

  void InferShape(framework::InferShapeContext *ctx) const override {
    PADDLE_ENFORCE(ctx->HasInput("X"), "SplitSelectedRowsOp must has input X.");
    PADDLE_ENFORCE(ctx->HasOutputs("Out"),
                   "SplitSelectedRowsOp must has output Out.");
  }
};

class SplitSelectedRowsOpInferVarType : public framework::VarTypeInference {
 public:
  void operator()(const framework::OpDesc &op_desc,
                  framework::BlockDesc *block) const override {
    for (auto &out_var : op_desc.Output("Out")) {
      block->Var(out_var)->SetType(framework::proto::VarType::SELECTED_ROWS);
    }
  }
};

class SplitSelectedRowsGradMaker : public framework::SingleGradOpDescMaker {
 public:
  using framework::SingleGradOpDescMaker::SingleGradOpDescMaker;

 protected:
  std::unique_ptr<framework::OpDesc> Apply() const override {
    auto *grad_op = new framework::OpDesc();
    grad_op->SetType("sum");
    grad_op->SetInput("X", OutputGrad("Out"));
    grad_op->SetOutput("Out", InputGrad("X"));
    grad_op->SetAttrMap(Attrs());
    return std::unique_ptr<framework::OpDesc>(grad_op);
  }
};

}  // namespace operators
}  // namespace paddle

namespace ops = paddle::operators;
REGISTER_OPERATOR(split_selected_rows, ops::SplitSelectedRowsOp,
                  ops::SplitSelectedRowsOpMaker,
                  ops::SplitSelectedRowsGradMaker,
                  ops::SplitSelectedRowsOpInferVarType);
REGISTER_OP_CPU_KERNEL(
    split_selected_rows,
    ops::SplitSelectedRowsOpKernel<paddle::platform::CPUPlace, float>);
