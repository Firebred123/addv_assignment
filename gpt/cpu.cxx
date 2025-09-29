#include "cpu.h"
#include <iostream>
#include <sstream>

using namespace sc_core;
using namespace std;

void CPU::b_transport(tlm::tlm_generic_payload &trans, sc_time &delay) {
  // Convert incoming payload to string command
  std::string cmd(reinterpret_cast<char *>(trans.get_data_ptr()),
                  trans.get_data_length());
  std::istringstream iss(cmd);
  std::string op;

  int x, y;

  // Note: input format like "add 4 36" or "write 100 0xfff0"
  iss >> op;

  // iss >> op >> x >> std::hex >> y;
  if (op == "write") {
    // For write: x is decimal, y is hex (address)
    iss >> x >> std::hex >> y;
  } else {
    // For other ops: both are decimal
    iss >> x >> y;
  }

  int result = 0;
  sc_time exec_time = SC_ZERO_TIME;

  if (op == "add") {
    // cout << x << ' ' << y << "pkl\n";
    result = x + y;
    exec_time = sc_time(10, SC_NS);
  } else if (op == "sub") {
    // cout << x << ' ' << y << "pkl\n";
    result = x - y;
    exec_time = sc_time(11, SC_NS);
  } else if (op == "eq") {
    // cout << x << ' ' << y << "pkl\n";
    result = (x == y);
    exec_time = sc_time(4, SC_NS);
  } else if (op == "rem") {
    // cout << x << ' ' << y << "pkl\n";
    result = x % y;
    exec_time = sc_time(15, SC_NS);
  } else if (op == "write") {
    // Prepare payload for memory
    tlm::tlm_generic_payload mem_trans;
    mem_trans.set_command(tlm::TLM_WRITE_COMMAND);
    mem_trans.set_address(y);
    mem_trans.set_data_ptr(reinterpret_cast<unsigned char *>(&x));
    mem_trans.set_data_length(sizeof(int));

    // Proper nb_transport_fw call
    tlm::tlm_phase phase = tlm::BEGIN_REQ;
    sc_time mem_delay = SC_ZERO_TIME;

    socket_to_mem->nb_transport_fw(mem_trans, phase, mem_delay);

    exec_time = SC_ZERO_TIME; // async write, delay modeled in memory
    cout << "CPU: Forwarded write(" << x << ") to address 0x" << std::hex << y
         << endl;
  }

  delay += exec_time;

  // Print result (except for writes, which are handled by memory)
  if (op != "write") {
    cout << "CPU executed: " << cmd << " -> result " << result << " at "
         << sc_time_stamp() + delay << endl;
  }
}
