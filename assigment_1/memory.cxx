// Memory.cpp — corrected (uses incoming 'delay' as arrival offset)
#include "memory.h"
#include <cstring> // memcpy
#include <iostream>

using namespace sc_core;
using namespace std;

tlm::tlm_sync_enum Memory::nb_transport_fw(tlm::tlm_generic_payload &trans,
                                           tlm::tlm_phase &phase,
                                           sc_core::sc_time &delay) {
  // Only accept BEGIN_REQ in this simple model
  if (phase != tlm::BEGIN_REQ) {
    trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
    return tlm::TLM_COMPLETED;
  }

  uint64_t addr = trans.get_address();
  unsigned int len = trans.get_data_length();
  unsigned char *orig_data_ptr = trans.get_data_ptr();

  const sc_time write_latency = sc_time(50, SC_NS);
  sc_time now = sc_time_stamp();

  // capture incoming annotated offset (time from 'now' when request logically
  // arrives)
  sc_time delay_in = delay;

  // Make a safe heap copy of the transaction + data
  tlm::tlm_generic_payload *trans_ptr = new tlm::tlm_generic_payload();
  unsigned char *data_copy = nullptr;
  if (len > 0) {
    data_copy = new unsigned char[len];
    std::memcpy(data_copy, orig_data_ptr, len);
    trans_ptr->set_data_ptr(data_copy);
    trans_ptr->set_data_length(len);
  } else {
    trans_ptr->set_data_ptr(nullptr);
    trans_ptr->set_data_length(0);
  }
  trans_ptr->set_command(trans.get_command());
  trans_ptr->set_address(addr);

  // Arrival time = now + delay_in
  sc_time arrival = now + delay_in;

  // The write should start at the later of arrival or current busy_end_time
  sc_time start_time = (busy_end_time > arrival) ? busy_end_time : arrival;
  sc_time completion_time = start_time + write_latency;

  // How long the initiator must wait from 'now' to reach the ack (start_time)
  sc_time delay_out = start_time - now;

  // Schedule a worker that waits until completion_time and executes do_write
  // Worker will: wait(delay_out + write_latency) => arrive at completion_time
  sc_spawn(
      [this, trans_ptr, delay_out, write_latency]() {
        // first wait until the start_time relative to now: wait(delay_out)
        wait(delay_out);
        // then perform the write latency
        wait(write_latency);
        // now we're at completion_time: commit and notify
        do_write(trans_ptr);
      },
      sc_gen_unique_name("mem_scheduled_write"));

  // update busy_end_time to reflect this new write
  busy_end_time = completion_time;

  // Tell initiator how long to wait from now before ack — this is the annotated
  // delay
  delay = delay_out;
  return tlm::TLM_ACCEPTED;
}

void Memory::do_write(tlm::tlm_generic_payload *trans_ptr) {
  uint64_t addr = trans_ptr->get_address();
  unsigned char *data_ptr = trans_ptr->get_data_ptr();

  if (data_ptr && trans_ptr->get_data_length() >= sizeof(int)) {
    int val = 0;
    std::memcpy(&val, data_ptr, sizeof(int));
    mem[addr] = val;
    cout << "Memory: completed write(" << val << ") to 0x" << std::hex << addr
         << " at " << sc_time_stamp() << endl;
  } else {
    cout << "Memory: write with invalid size/address at " << sc_time_stamp()
         << endl;
  }

  // set an OK response before callback
  trans_ptr->set_response_status(tlm::TLM_OK_RESPONSE);

  // notify initiator via backward path (BEGIN_RESP)
  tlm::tlm_phase bw_phase = tlm::BEGIN_RESP;
  sc_time bw_delay = SC_ZERO_TIME;
  socket->nb_transport_bw(*trans_ptr, bw_phase, bw_delay);

  // cleanup
  unsigned char *d = trans_ptr->get_data_ptr();
  if (d)
    delete[] d;
  delete trans_ptr;
}
