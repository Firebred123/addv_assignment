#include "cpu.h"
#include "memory.h"
#include "software.h"
#include <systemc>

using namespace sc_core; // fix sc_start()

int sc_main(int argc, char *argv[]) {
  Software sw("sw");
  CPU cpu("cpu");
  Memory mem("mem");

  sw.socket.bind(cpu.socket_from_sw);
  cpu.socket_to_mem.bind(mem.socket);

  sc_start(); // OK now

  return 0;
}
