/*
 * Copyright 2019 SiFive, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You should have received a copy of LICENSE.Apache2 along with
 * this software. If not, you may obtain a copy at
 *
 *    https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ssa.h"

struct PassSweep {
  TermStream stream;
  PassSweep(TargetScope &scope) : stream(scope) { }
};

static void redux_sweep(PassSweep &p, Redux *r) {
  r->update(p.stream.map());
  for (auto x : r->args) ++p.stream[x]->meta;
}

void RArg::pass_sweep(PassSweep &p) {
}

void RLit::pass_sweep(PassSweep &p) {
}

void RApp::pass_sweep(PassSweep &p) {
  redux_sweep(p, this);
}

void RPrim::pass_sweep(PassSweep &p) {
  redux_sweep(p, this);
}

void RGet::pass_sweep(PassSweep &p) {
  redux_sweep(p, this);
}

void RDes::pass_sweep(PassSweep &p) {
  redux_sweep(p, this);
}

void RCon::pass_sweep(PassSweep &p) {
  redux_sweep(p, this);
}

void RFun::pass_sweep(PassSweep &p) {
  CheckPoint cp = p.stream.begin();
  for (auto &x : terms) {
    if (x->meta) {
      p.stream.discard();
    } else {
      Term *t = x.get();
      p.stream.transfer(std::move(x));
      t->pass_sweep(p);
    }
  }
  update(p.stream.map());
  ++p.stream[output]->meta;
  terms = p.stream.end(cp);
}

std::unique_ptr<Term> Term::pass_sweep(std::unique_ptr<Term> term) {
  TargetScope scope;
  PassSweep pass(scope);
  pass.stream.transfer(std::move(term));
  scope[0]->pass_sweep(pass);
  return scope.finish();
}
