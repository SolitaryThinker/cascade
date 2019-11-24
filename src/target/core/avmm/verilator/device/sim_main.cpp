#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netinet/tcp.h>

#include "Vprogram_logic.h"
#include "verilated.h"
int connect_to_cascade(char *addr, int port);

int main(int argc, char** argv, char** env) {
  //Verilated::commandArgs(argc, argv);
  Vprogram_logic* top = new Vprogram_logic;

  vluint64_t main_time = 0;

  printf("addr: %s\n", argv[1]);
  int port = atoi(argv[2]);
  printf("port: %d\n", port);

  int sock = connect_to_cascade(argv[1], port);
  FILE* sock_stream = fdopen(sock, "a+");
  top->reset = 0;
  char temp = 0;
  char buf[4];

  // initialize variables
  top->s0_read = 0;
  top->s0_write = 0;

  top->s0_address = 0;
  top->s0_writedata = 0;
  top->s0_readdata = 0;

  top->clk = 0;
  top->eval();
  top->clk = 1;
  top->eval();

  while (!Verilated::gotFinish()) { 
    //if (main_time % 1000000 == 0)
      //printf("progress %ld, %x\n", main_time, top->s0_waitrequest);
    if (main_time % 2 == 0) {
      //printf("setting to 1\n");
      top->clk = 1;
      //top->eval();

      if ((top->s0_read || top->s0_write) && !top->s0_waitrequest) {
        //printf("progress %ld\n", main_time);
        if (top->s0_read) {
          char *data_out = (char *)&top->s0_readdata;
          //printf("read request=====\n");
          buf[0] = data_out[3];
          buf[1] = data_out[2];
          buf[2] = data_out[1];
          buf[3] = data_out[0];
          //dprintf(sock, "%c%c%c%c", data_out[3], data_out[2], data_out[1], data_out[0]);
          int c = send(sock, buf, 4, 0);
          //if (c == -1) {
            //perror("send");
            //exit(1);
          //}
          //printf("after send %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);
        }
        top->s0_read = 0;
        top->s0_write = 0;

        top->clk = 0;
        top->eval();
        top->clk = 1;
        top->eval();
        //printf("\n");
      }
      if (!(top->s0_read || top->s0_write)) {
        //printf("no request=====\n");
        int c = recv(sock, buf, 3, 0);
        //if (c == -1) {
          //perror("recv");
          //exit(1);
        //}
        //printf("after recv %x %x %x\n", buf[0], buf[1], buf[2]);
        ((char *)&top->s0_address)[1] = buf[1];
        ((char *)&top->s0_address)[0] = buf[2];
        if (feof(sock_stream)) {
          printf("eof!\n");
        } else {
          top->s0_read = (buf[0] & 0x2) ? 1 : 0;
          top->s0_write = (buf[0] & 0x1) ? 1 : 0;
          if (buf[0] & 0x1) {
            //printf("write request======\n");
            char *data_in = (char *)&top->s0_writedata;
            //fscanf(sock_stream, "%c%c%c%c", data_in[3], data_in[2], data_in[1], data_in[0]);
            int c = recv(sock, buf, 4, 0);
            //if (c == -1) {
              //perror("recv");
              //exit(1);
            //}
            data_in[0] = buf[3];
            data_in[1] = buf[2];
            data_in[2] = buf[1];
            data_in[3] = buf[0];
            //printf("after recv %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);
          }
          //printf("\n");
        }
      }
      //printf("clk: %d\n", top->clk);
      //printf("s0_write: %d\n", top->s0_write);
      //printf("s0_read: %d\n", top->s0_read);
      ////std::cout<<"clk: " << top->clk << std::endl;
      ////std::cout<<"s0_read: " << top->s0_read << std::endl;
      ////std::cout<<"s0_write: " << top->s0_write << std::endl;
      //std::cout<<"s0_writedata: " << top->s0_writedata << std::endl;
      //std::cout<<"s0_readdata: " << top->s0_readdata << std::endl;
      //std::cout<<"s0_waitrequest: " << top->s0_waitrequest << std::endl;
    }
    if (main_time % 2 == 1) {
      top->clk = 0;
    }
    top->eval(); 
    main_time++;
  }
  delete top;
  exit(0);
}

int connect_to_cascade(char *addr, int port) {
  int opt = 1;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;
  char buffer[1024] = {0};

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  // Convert IPv4 and IPv6 addresses from text to binary form
  if(inet_pton(AF_INET, addr, &serv_addr.sin_addr)<=0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("\nConnection Failed \n");
    return -1;
  }
  if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt))) {
    perror("setsockopt");
    return -1;
  }

  return sock;
}
