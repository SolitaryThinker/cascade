// Copyright 2017-2019 VMware, Inc.
// SPDX-License-Identifier: BSD-2-Clause
//
// The BSD-2 license (the License) set forth below applies to all parts of the
// Cascade project.  You may not use this file except in compliance with the
// License.
//
// BSD-2 License
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "target/core/avmm/verilator/verilator_compiler.h"

#include <fstream>
#include "common/system.h"

using namespace std;

namespace cascade {

VerilatorCompiler::VerilatorCompiler() : AvmmCompiler<uint32_t>() {
  // TODO...
}

VerilatorCompiler::~VerilatorCompiler() {
  // TODO...
}

VerilatorLogic* VerilatorCompiler::build(Interface* interface, ModuleDeclaration* md, size_t slot) {
  // TODO...
  return new VerilatorLogic(interface, md, slot);
}

bool VerilatorCompiler::compile(const string& text, mutex& lock) {
  (void) lock;
  get_compiler()->schedule_state_safe_interrupt([this, &text]{
      ofstream ofs(System::src_root() + "/src/target/core/avmm/verilator/device/program_logic.v");
      ofs << text << endl;
      ofs.close();
      std::cout << "compile end\n";
    // TODO...
  });
  return true;
}

void VerilatorCompiler::stop_compile() {
  // Does nothing. 
}

} // namespace cascade
