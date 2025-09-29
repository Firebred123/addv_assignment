#ifndef CPU_H
#define CPU_H

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

struct CPU : sc_core::sc_module {
  tlm_utils::simple_target_socket<CPU> socket_from_sw;
  tlm_utils::simple_initiator_socket<CPU> socket_to_mem;

  SC_CTOR(CPU)
      : socket_from_sw("socket_from_sw"), socket_to_mem("socket_to_mem") {
    socket_from_sw.register_b_transport(this, &CPU::b_transport);
  }

  void b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay);
};

#endif
