#ifndef SOFTWARE_H
#define SOFTWARE_H

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>

struct Software : sc_core::sc_module {
  tlm_utils::simple_initiator_socket<Software> socket;

  SC_CTOR(Software) : socket("socket") { SC_THREAD(run); }

  void run();
};

#endif
