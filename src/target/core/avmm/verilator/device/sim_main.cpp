#include <iostream>
#include "Vprogram_logic.h"
#include "verilated.h"
int main(int argc, char** argv, char** env) {
  std::cout<<"main\n";
  Verilated::commandArgs(argc, argv);
  Vprogram_logic* top = new Vprogram_logic;
  std::cout<<"before while\n";
  while (!Verilated::gotFinish()) { 
    std::cout<<"in while\n";

    top->eval(); }
    std::cout<<"after while\n";
  delete top;
  exit(0);
}
