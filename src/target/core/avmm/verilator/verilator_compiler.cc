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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "common/system.h"
#define PORT 8080

using namespace std;
int bind_and_listen();
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

  int sock = bind_and_listen();

  int valread;
  char buffer[1024] = {0};
  char *hello = "Hello from server";

  printf("reading\n");
  valread = ::read( sock , buffer, 1024);
  printf("%s\n",buffer );
  printf("sending\n");
  send(sock , hello , strlen(hello) , 0 );
  printf("Hello message sent\n");


  return new VerilatorLogic(interface, md, slot, sock);
}

bool VerilatorCompiler::compile(const string& text, mutex& lock) {
  (void) lock;
      std::cout << "compile begin\n";
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

int bind_and_listen() {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
      perror("socket failed");
      exit(EXIT_FAILURE);
  }

  // Forcefully attaching socket to the port 8080
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                &opt, sizeof(opt)))
  {
      perror("setsockopt");
      exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons( PORT );

  // Forcefully attaching socket to the port 8080
  if (bind(server_fd, (struct sockaddr *)&address,
                               sizeof(address))<0)
  {
      perror("bind failed");
      exit(EXIT_FAILURE);
  }
  printf("listening\n");
  if (listen(server_fd, 3) < 0)
  {
      perror("listen");
      exit(EXIT_FAILURE);
  }
  printf("accepting\n");
  if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                     (socklen_t*)&addrlen))<0)
  {
      perror("accept");
      exit(EXIT_FAILURE);
  }

  return new_socket;
}

