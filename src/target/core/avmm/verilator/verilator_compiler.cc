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
#include <stdio.h>
#include <sys/wait.h>


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
  std::cout << "building VerilatorLogic\n";
  int sock = 0;

  //int sock = bind_and_listen();

  return new VerilatorLogic(interface, md, slot, &sock_stream_);
}

bool VerilatorCompiler::compile(const string& text, mutex& lock) {
  (void) lock;

  std::cout << "compile begin\n";
get_compiler()->schedule_state_safe_interrupt([this, &text]{

  ofstream ofs(System::src_root() + "/src/target/core/avmm/verilator/device/program_logic.v");
  ofs << text << endl;
  ofs.close();
  std::cout << "compile end\n";
  if (!compile_verilator()) {
    exit(1);
  }
  sock_stream_.initialize_stream();
  //if (!start_verilator()) {
    //exit(1);
  //}

    // TODO...
  });
  //return false;
  return true;
}

void VerilatorCompiler::stop_compile() {
  // Does nothing. 
}

inline bool VerilatorCompiler::compile_verilator() {
  int pid = -1;
  if ((pid = fork()) == 0) {
    // child
    if (chdir((System::src_root() + "/src/target/core/avmm/verilator/device/").c_str()) == -1) {
      perror("chdir failed");
    }
    int dev_null = open("/dev/null", O_WRONLY);
    if (freopen("/dev/null", "a", stdout) == NULL) {
      perror("freopen dev null failed");
    }
    if (dup2(fileno(stdout), STDERR_FILENO) == -1) {
      perror("dup2 failed");
    }
    if (execlp("make", NULL) == -1) {
      perror("exec failed");
    }
  } else if (pid > 0) {
    int status;
    // parent
    if (waitpid(pid, &status, 0) == -1) {
      perror("waitpid failed");
    }
    if (WEXITSTATUS(status) == 0) {
      std::cout<<"make success\n";
    } else {
      std::cout<<"make failed\n";
      return false;
    }
  } else {
    perror("fork failed");
    return false;
  }
  return true;
}

inline bool VerilatorCompiler::start_verilator() {
  int pid = -1;
  if ((pid = fork()) == 0) {
    // child
    if (chdir((System::src_root() + "/src/target/core/avmm/verilator/device/").c_str()) == -1) {
      perror("chdir failed");
    }
    int dev_null = open("/dev/null", O_WRONLY);
    if (freopen("/dev/null", "a", stdout) == NULL) {
      perror("freopen dev null failed");
    }
    if (dup2(fileno(stdout), STDERR_FILENO) == -1) {
      perror("dup2 failed");
    }
    if (execlp("make", NULL) == -1) {
      perror("exec failed");
    }
  } else if (pid > 0) {
    int status;
    // parent
    if (waitpid(pid, &status, 0) == -1) {
      perror("waitpid failed");
    }
    if (WEXITSTATUS(status) == 0) {
      std::cout<<"make success\n";
    } else {
      std::cout<<"make failed\n";
      return false;
    }
  } else {
    perror("fork failed");
    return false;
  }
  return true;
}


} // namespace cascade
