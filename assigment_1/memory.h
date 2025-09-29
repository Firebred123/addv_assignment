#ifndef MEMORY_H
#define MEMORY_H

#include <map>
#include <queue>
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>

struct Memory : sc_core::sc_module {
  tlm_utils::simple_target_socket<Memory> socket;
  std::map<uint64_t, int> mem;

  // track when memory becomes free (simtime). initial = 0.
  sc_core::sc_time busy_end_time;

  SC_CTOR(Memory) : socket("socket"), busy_end_time(sc_core::SC_ZERO_TIME) {
    socket.register_nb_transport_fw(this, &Memory::nb_transport_fw);
  }

  // non-blocking forward path (initiator -> target)
  tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload &trans,
                                     tlm::tlm_phase &phase,
                                     sc_core::sc_time &delay);

private:
  // helper to perform the real write (runs in spawned process)
  void do_write(tlm::tlm_generic_payload *trans_ptr);
};

#endif
