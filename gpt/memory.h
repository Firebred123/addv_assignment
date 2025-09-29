#ifndef MEMORY_H
#define MEMORY_H

#include <map>
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>

struct Memory : sc_core::sc_module {
  tlm_utils::simple_target_socket<Memory> socket;
  std::map<uint64_t, int> mem;

  SC_CTOR(Memory) : socket("socket") {
    socket.register_nb_transport_fw(this, &Memory::nb_transport_fw);
  }

  tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload &trans,
                                     tlm::tlm_phase &phase,
                                     sc_core::sc_time &delay);
};

#endif
