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

#include "target/core/avmm/de10/quartus_server.h"

#include <fstream>
#include <sstream>
#include "common/sockserver.h"
#include "common/sockstream.h"
#include "common/system.h"

using namespace std;

namespace cascade {

QuartusServer::QuartusServer() : Thread() { 
  set_cache_path("/tmp/quartus_cache/");
  set_quartus_path("");
  set_port(9900);

  busy_ = false;
}

QuartusServer& QuartusServer::set_cache_path(const string& path) {
  cache_path_ = path;
  return *this;
}

QuartusServer& QuartusServer::set_quartus_path(const string& path) {
  quartus_path_ = path;
  return *this;
}

QuartusServer& QuartusServer::set_port(uint32_t port) {
  port_ = port;
  return *this;
}

bool QuartusServer::error() const {
  // Return true if we can't locate any of the necessary quartus components
  if (System::execute("ls " + quartus_path_ + "/sopc_builder/bin/qsys-generate > /dev/null") != 0) {
    return true;
  }
  if (System::execute("ls " + quartus_path_ + "/bin/quartus_map > /dev/null") != 0) {
    return true;
  }
  if (System::execute("ls " + quartus_path_ + "/bin/quartus_fit > /dev/null") != 0) {
    return true;
  }
  if (System::execute("ls " + quartus_path_ + "/bin/quartus_asm > /dev/null") != 0) {
    return true;
  }
  if (System::execute("ls " + quartus_path_ + "/bin/quartus_pgm > /dev/null") != 0) {
    return true;
  }
  return false;
}

void QuartusServer::run_logic() {
  // Initialize thread pool and comilation cache
  init_pool();
  init_cache();

  // Return immediately if we can't create a sockserver
  sockserver server(port_, 8);
  if (server.error()) {
    pool_.stop_now();
    return;
  }

  fd_set master_set;
  FD_ZERO(&master_set);
  FD_SET(server.descriptor(), &master_set);

  fd_set read_set;
  FD_ZERO(&read_set);

  struct timeval timeout = {1, 0};

  while (!stop_requested()) {
    read_set = master_set;
    select(server.descriptor()+1, &read_set, nullptr, nullptr, &timeout);
    if (!FD_ISSET(server.descriptor(), &read_set)) {
      continue;
    }

    auto* sock = server.accept();
    const auto rpc = static_cast<QuartusServer::Rpc>(sock->get());
    
    // At most one compilation thread can be active at once. Issue kill-alls
    // until this is no longer the case.
    if (rpc == Rpc::KILL_ALL) {
      while (busy_) {
        kill_all();
        this_thread::sleep_for(chrono::seconds(1));
      }
      sock->put(static_cast<uint8_t>(Rpc::OKAY));
      sock->flush();
      delete sock;
    } 
    // Kill the one compilation thread if necessary and then fire off a new thread to
    // attempt a recompilation. When the new thread is finished it will reset the busy
    // flag.
    else if (rpc == Rpc::COMPILE) {
      while (busy_) {
        kill_all();
        this_thread::sleep_for(chrono::seconds(1));
      }
      sock->put(static_cast<uint8_t>(Rpc::OKAY));
      sock->flush();
      busy_ = true;

      pool_.insert([this, sock]{
        string text = "";
        getline(*sock, text, '\0'); 
        const auto res = compile(text);
        sock->put(static_cast<uint8_t>(res ? Rpc::OKAY : Rpc::ERROR));
        sock->flush();

        if (res) {
          sock->get();
          
          const auto itr = cache_.find(text);
          assert(itr != cache_.end());
          ifstream ifs(cache_path_ + "/" + itr->second, ios::binary);
          
          stringstream rbf;
          rbf << ifs.rdbuf();
          uint32_t len = rbf.str().length();

          sock->write(reinterpret_cast<const char*>(&len), sizeof(len));
          sock->write(rbf.str().c_str(), len);
          sock->flush();    

          sock->get();
        }

        busy_ = false;
        delete sock;
      });
    }
    // Unrecognized RPC
    else {
      assert(false);
      delete sock;
    }
  }

  // Stop the thread pool
  pool_.stop_now();
}

void QuartusServer::init_pool() {
  // We have the invariant that there is exactly one compile thread out at any
  // given time, so no need to prime the pool with anything more than that.
  pool_.stop_now();
  pool_.set_num_threads(1);
  pool_.run();
}

void QuartusServer::init_cache() {
  // Create the cache if it doesn't already exist
  System::execute("mkdir -p " + cache_path_);
  System::execute("touch " + cache_path_ + "/index.txt");

  // Read cache into memory
  ifstream ifs(cache_path_ + "/index.txt");
  cache_.clear();
  while (true) {
    string text;
    getline(ifs, text, '\0');
    if (ifs.eof()) {
      break;
    } 

    string path;
    getline(ifs, path, '\0');
    cache_.insert(make_pair(text, path));
  } 
}

void QuartusServer::kill_all() {
  // Note that we don't kill anything downstream of place and route as these
  // passess all run to completion in short order.
  System::execute("killall java > /dev/null 2>&1");
  System::execute("killall quartus_map > /dev/null 2>&1");
  System::execute("killall quartus_fit > /dev/null 2>&1");
}

bool QuartusServer::compile(const std::string& text) {
  // Nothing to do if this code is already in the cache. 
  if (cache_.find(text) != cache_.end()) {
    return true;
  }

  // Otherwise, compile the code and add a new entry to the cache
  ofstream ofs(System::src_root() + "/src/target/core/avmm/de10/device/ip/program_logic.v");
  ofs << text << endl;
  ofs.flush();
  if (System::execute(quartus_path_ + "/sopc_builder/bin/qsys-generate " + System::src_root() + "/src/target/core/avmm/de10/device/soc_system.qsys --synthesis=VERILOG") != 0) {
    return false;
  } 
  if (System::execute(quartus_path_ + "/bin/quartus_map " + System::src_root() + "/src/target/core/avmm/de10/device/DE10_NANO_SoC_GHRD.qpf") != 0) {
    return false;
  } 
  if (System::execute(quartus_path_ + "/bin/quartus_fit " + System::src_root() + "/src/target/core/avmm/de10/device/DE10_NANO_SoC_GHRD.qpf") != 0) {
    return false;
  } 

  // If we've made it this far, we're bound for success. Kill all can't stop us.
  if (System::execute(quartus_path_ + "/bin/quartus_asm " + System::src_root() + "/src/target/core/avmm/de10/device/DE10_NANO_SoC_GHRD.qpf") != 0) {
    return false;
  }
  if (System::execute(quartus_path_ + "/bin/quartus_cpf -c " + System::src_root() + "/src/target/core/avmm/de10/device/sof2rbf.cof") != 0) {
    return false;
  }
  stringstream ss;
  ss << "bitstream_" << cache_.size() << ".rbf";
  const auto file = ss.str();
  System::execute("cp " + System::src_root() + "/src/target/core/avmm/de10/device/output_files/DE10_NANO_SoC_GHRD.rbf " + cache_path_ + "/" + file);

  ofstream ofs2(cache_path_ + "/index.txt", ios::app);
  ofs2 << text << '\0' << file << '\0';
  ofs2.flush();

  cache_[text] = file;

  return true;
}

} // namespace cascade
