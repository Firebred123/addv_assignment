
#ifndef CPU_H
#define CPU_H

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

struct CPU : sc_core::sc_module {
  // software -> CPU (commands)
  tlm_utils::simple_target_socket<CPU> socket_from_sw;
  // CPU -> memory (requests)
  tlm_utils::simple_initiator_socket<CPU> socket_to_mem;
  int outstanding_writes = 0;
  sc_core::sc_event writes_done_event;

  SC_CTOR(CPU)
      : socket_from_sw("socket_from_sw"), socket_to_mem("socket_to_mem") {
    // software -> CPU uses blocking transport
    socket_from_sw.register_b_transport(this, &CPU::b_transport);

    // register backward path handler so memory can call back with
    // END/BEGIN_RESP
    socket_to_mem.register_nb_transport_bw(this, &CPU::nb_transport_bw);
  }

  // forward path: software tells CPU to execute instruction (blocking call)
  void b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay);

  // backward path: memory calls this to indicate response/completion
  tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload &trans,
                                     tlm::tlm_phase &phase,
                                     sc_core::sc_time &delay);

  // DMI invalidation (not used, but must be present)
  void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                 sc_dt::uint64 end_range);
};

#endif
