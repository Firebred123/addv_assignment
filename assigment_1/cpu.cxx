#include "cpu.h"
#include <iomanip>
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

  int x = 0, y = 0;

  // Note: input format like "add 4 36" or "write 100 0xfff0" or "sync"
  iss >> op;

  // --- handle special "sync" command: block until outstanding writes finish
  // ---
  if (op == "sync") {
    if (outstanding_writes > 0) {
      // wait until memory has sent all BEGIN_RESP callbacks
      wait(writes_done_event);
    }
    // nothing to advance in 'delay' (we advanced simulation time inside wait)
    return;
  }

  // parse operands
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
    result = x + y;
    exec_time = sc_time(10, SC_NS);
  } else if (op == "sub") {
    result = x - y;
    exec_time = sc_time(11, SC_NS);
  } else if (op == "eq") {
    result = (x == y);
    exec_time = sc_time(4, SC_NS);
  } else if (op == "rem") {
    result = x % y;
    exec_time = sc_time(15, SC_NS);
  } else if (op == "write") {
    // Prepare payload for memory
    tlm::tlm_generic_payload mem_trans;
    mem_trans.set_command(tlm::TLM_WRITE_COMMAND);
    mem_trans.set_address(static_cast<uint64_t>(y));
    mem_trans.set_data_ptr(reinterpret_cast<unsigned char *>(&x));
    mem_trans.set_data_length(sizeof(int));

    tlm::tlm_phase phase = tlm::BEGIN_REQ;

    // Annotate the request with the CPU's accumulated delay so memory knows
    // when this request logically arrives (arrival = now + mem_delay)
    sc_core::sc_time mem_delay = delay;

    // Forward transaction (non-blocking)
    tlm::tlm_sync_enum status =
        socket_to_mem->nb_transport_fw(mem_trans, phase, mem_delay);

    // Mark as outstanding (we expect a BEGIN_RESP later)
    outstanding_writes++;

    // Wait for the annotated delay returned/modified by target.
    // mem_delay now represents how long the initiator should wait from 'now'
    // until the request is acknowledged (start_time - now).
    if (mem_delay != sc_core::SC_ZERO_TIME)
      wait(mem_delay);

    // We consumed the CPU delay (we advanced simulation), reset to avoid
    // double-counting
    delay = sc_core::SC_ZERO_TIME;

    // Acknowledgement (logical time) - CPU time now corresponds to ack time
    cout << "CPU: Acknowledged write(" << x << ") to 0x" << std::hex << y
         << " at " << sc_time_stamp() << endl;
  }

  // cout << "for debug --- instr " << op << " delay- " << delay << " + "
  //      << exec_time << '\n';

  // Advance this instruction's own execution time (non-memory ops)
  delay += exec_time;

  // Print result (except for writes, which are handled by memory)
  if (op != "write") {
    cout << "CPU executed: " << cmd << " -> result " << result << " at "
         << sc_time_stamp() + delay << endl;
  }
}

// ---------------- Backward path ----------------
tlm::tlm_sync_enum CPU::nb_transport_bw(tlm::tlm_generic_payload &trans,
                                        tlm::tlm_phase &phase,
                                        sc_core::sc_time &delay) {
  // Memory calls back with BEGIN_RESP when a write actually completes.
  if (phase == tlm::BEGIN_RESP) {
    cout << "CPU: received BEGIN_RESP (write completed) at " << sc_time_stamp()
         << endl;

    // bookkeeping: decrement outstanding writes and notify if zero
    if (outstanding_writes > 0)
      --outstanding_writes;
    if (outstanding_writes == 0) {
      writes_done_event.notify();
    }

    // Indicate we've handled the response completely
    return tlm::TLM_COMPLETED;
  }

  // Default: accept other phases
  return tlm::TLM_ACCEPTED;
}

void CPU::invalidate_direct_mem_ptr(sc_dt::uint64 /*start_range*/,
                                    sc_dt::uint64 /*end_range*/) {
  // no DMI in this model
}
