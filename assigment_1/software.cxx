
#include "software.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace sc_core;
using namespace std;

void Software::run() {
  std::vector<std::string> commands = {
      "add 4 36",        "eq 49 55",        "rem 20 2",    "write 100 0xfff0",
      "sub 9 12",        "write 21 0xff00", "add 12 1000", "rem 5 8",
      "write 21 0xff40", "rem 100 7",       "sub 125 25",  "eq 47 47",
      "add 5 6",         "write 21 0xff80"};

  cout << "================ Simulation Start ================\n";

  sc_time delay = SC_ZERO_TIME;

  for (auto &cmd : commands) {
    // allocate so payload pointer remains valid for the duration of b_transport
    std::string *data = new std::string(cmd);

    tlm::tlm_generic_payload trans;
    trans.set_command(tlm::TLM_WRITE_COMMAND);
    trans.set_data_ptr(reinterpret_cast<unsigned char *>(data->data()));
    trans.set_data_length(data->size());

    // blocking call into CPU (which will forward to memory etc.)
    socket->b_transport(trans, delay);

    // cleanup after the blocking call returns
    delete data;
  }

  // --- ensure all outstanding writes have completed by asking CPU to "sync"
  // ---
  {
    std::string *sync_cmd = new std::string("sync");
    tlm::tlm_generic_payload sync_trans;
    sync_trans.set_command(tlm::TLM_WRITE_COMMAND);
    sync_trans.set_data_ptr(
        reinterpret_cast<unsigned char *>(sync_cmd->data()));
    sync_trans.set_data_length(sync_cmd->size());

    sc_time sync_delay = SC_ZERO_TIME;
    socket->b_transport(sync_trans, sync_delay);
    delete sync_cmd;
  }

  cout << "\n================ Simulation End ==================\n";
  cout << "[SW] Total simulated time: " << sc_time_stamp() << "\n";
  cout << "=================================================\n";
}
