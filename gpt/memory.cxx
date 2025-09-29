#include "memory.h"
#include <iostream>

using namespace sc_core;
using namespace std;

tlm::tlm_sync_enum Memory::nb_transport_fw(tlm::tlm_generic_payload &trans,
                                           tlm::tlm_phase &phase,
                                           sc_time &delay) {

  uint64_t addr = trans.get_address();
  int *data = reinterpret_cast<int *>(trans.get_data_ptr());

  sc_time write_time = sc_time(50, SC_NS);
  wait(write_time); // model write delay

  mem[addr] = *data;
  cout << "Memory: Write " << *data << " at address 0x" << hex << addr
       << " completed at " << sc_time_stamp() << endl;

  return tlm::TLM_COMPLETED;
}
