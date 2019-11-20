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

#include "target/core/avmm/verilator/verilator_logic.h"
#include "target/core/avmm/verilator/sockbuf.h"

namespace cascade {

VerilatorLogic::VerilatorLogic(Interface* interface, ModuleDeclaration* md, size_t slot, sockbuf* sock_stream) : AvmmLogic<uint32_t>(interface, md) {
  //printf("in VerilatorLogic\n");
  get_table()->set_read([slot, sock_stream](size_t index) {
      //printf("IN SET READsent\n");
    uint8_t bytes[7];
    const uint16_t vid = index | (slot << 12);
    bytes[0] = 2;
    bytes[1] = vid >> 8;
    bytes[2] = vid;
    //send(sock_stream, bytes, 7, 0);
    //::read(sock_stream, bytes, 4);
    sock_stream->sendn(reinterpret_cast<const char*>(bytes), 3);
    sock_stream->waitforn(reinterpret_cast<char*>(bytes), 4);
    //reqs->sputn(reinterpret_cast<const char*>(bytes), 3);
    //resps->waitforn(reinterpret_cast<char*>(bytes), 4);
    return (bytes[3]) | (bytes[2] << 8) | (bytes[1] << 16) | (bytes[0] << 24);
    // TODO...
    return 0;
  });
  get_table()->set_write([slot, sock_stream](size_t index, uint32_t val) {
      //printf("IN SET WRITE\n");
    uint8_t bytes[7];
    const uint16_t vid = index | (slot << 12);
    const uint32_t data = val;
    bytes[0] = 1;
    bytes[1] = vid >> 8;
    bytes[2] = vid;
    bytes[3] = data >> 24;
    bytes[4] = data >> 16;
    bytes[5] = data >> 8;
    bytes[6] = data;
    //send(sock_stream, bytes, 7, 0 );
    sock_stream->sendn(reinterpret_cast<const char*>(bytes), 7);
    //reqs->sputn(reinterpret_cast<const char*>(bytes), 7);
    // TODO...
  });
}

} // namespace cascade
