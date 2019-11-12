#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "Vprogram_logic.h"
#include "verilated.h"
int connect_to_cascade(char *addr, int port);

int main(int argc, char** argv, char** env) {
  std::cout<<"main\n";

  //Verilated::commandArgs(argc, argv);
  Vprogram_logic* top = new Vprogram_logic;

  std::cout<<"before while\n";
  vluint64_t main_time = 0;

  printf("addr: %s\n", argv[1]);
  int port = atoi(argv[2]);
  printf("port: %d\n", port);

  int sock = connect_to_cascade(argv[1], port);
  top->reset = 0;
  return 0;

  while (!Verilated::gotFinish()) { 
    if (main_time % 10 == 0) {
      top->clk = 1;
      if ((top->s0_read || top->s0_write) && !top->s0_waitrequest) {
        if (top->s0_read)
          printf("write some stuff\n");
        top->s0_read = 0;
        top->s0_write = 0;
      }
      if (!(top->s0_read || top->s0_write)) {
          printf("flush and scanf\n");
      }
      printf("clk: %d\n", top->clk);
      printf("s0_write: %d\n", top->s0_write);
      printf("s0_read: %d\n", top->s0_read);
      //std::cout<<"clk: " << top->clk << std::endl;
      //std::cout<<"s0_read: " << top->s0_read << std::endl;
      //std::cout<<"s0_write: " << top->s0_write << std::endl;
      std::cout<<"s0_writedata: " << top->s0_writedata << std::endl;
      std::cout<<"s0_readdata: " << top->s0_readdata << std::endl;
      std::cout<<"s0_waitrequest: " << top->s0_waitrequest << std::endl;
    }
    if (main_time % 10 == 6) {
      top->clk = 0;
    }


    //std::cout<<"before eval\n";
    top->eval(); 
    //std::cout<<"after eval\n";
    main_time++;
  }
    std::cout<<"after while\n";
  delete top;
  exit(0);
}

int connect_to_cascade(char *addr, int port) {
  int sock = 0, valread;
  struct sockaddr_in serv_addr;
  char *hello = "Hello from client";
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
  send(sock , hello , strlen(hello) , 0 );
  printf("Hello message sent\n");
  valread = read( sock , buffer, 1024);
  printf("%s\n",buffer );
  
}
