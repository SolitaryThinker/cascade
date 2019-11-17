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

#ifndef CASCADE_SRC_TARGET_CORE_AVMM_VERILATOR_SOCKBUF_H
#define CASCADE_SRC_TARGET_CORE_AVMM_VERILATOR_SOCKBUF_H

#include <streambuf>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netinet/tcp.h>

namespace cascade {

// FIFO with atomic puts and gets Peeking and put-backs are not supported

class sockbuf {
  public:
    // Constructors:
    sockbuf(int port);
    //~sockbuf();
    
    void initialize_stream();
    // Blocking Read:
    void waitforn(char* s, size_t count);
    void sendn(const char* s, size_t count);

  private:
    int socket_fd_ = -1;
    int port_;
    // Shared data buffer:
    std::queue<char *> data_;

    // Synchronization:
    std::mutex mut_;
    std::condition_variable cv_;
    
    // Get Area:
    //int_type uflow() override;
    //std::streamsize xsgetn(char_type* s, std::streamsize count) override;

    //// Put Area:
    //std::streamsize xsputn(const char_type* s, std::streamsize count) override;
};

inline sockbuf::sockbuf(int port) {
  port_ = port;
}

//inline sockbuf::~sockbuf() {
  //delete data_;
  //std::cout << "Final capacity: " << data_cap_ << std::endl;
//}

inline void sockbuf::waitforn(char* s, size_t count) {
  std::cout<<"sockbuf waiting\n";
  int c = ::read(socket_fd_, s, count);
  if (c != count) {
    perror("did not match");
    exit(1);
  }
}

inline void sockbuf::sendn(const char* s, size_t count) {
  // need to buffer in queue until stream is initialized
  if (socket_fd_ == -1) {
    std::cout<<"adding to queue\n";
    char *buffer = new char[count];
    std::copy(s, s+count, buffer);
    data_.push(buffer);
  } else {
    std::cout<<"sockbuf sending\n";
    for (int i = 0; i < count; i++)
      printf("%x\n", s[i]);
    printf("\n");
    int c = send(socket_fd_, s, count, 0);
    if (c != count) {
      perror("did not match");
      exit(1);
    }
    //fsync(socket_fd_);
  }
}

inline void sockbuf::initialize_stream() {
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
  address.sin_port = htons(port_);

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
  if (setsockopt(new_socket, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)))
  {
      perror("setsockopt");
      exit(EXIT_FAILURE);
  }
  socket_fd_ = new_socket;
  //while(1);

  //int valread;
  //char buffer[1024] = {0};
  char *hello = "Hello from server";

  //printf("reading\n");
  //valread = ::read( socket_fd_ , buffer, 1024);
  //printf("%s\n",buffer );
  //printf("sending\n");
  //send(socket_fd_ , hello , strlen(hello) , 0 );
  //fsync(socket_fd_);
  //printf("Hello message sent\n");
}

} // namespace cascade

#endif
