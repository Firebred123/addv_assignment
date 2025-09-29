
#include "software.h"
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace sc_core;
using namespace std;

void Software::run() {
  std::vector<std::string> commands = {
      "add 4 16",        "eq 49 55",        "rem 20 2",    "write 100 0xfff0",
      "sub 9 12",        "write 21 0xff00", "add 12 1000", "rem 5 8",
      "write 21 0xff40", "rem 100 7",       "sub 125 25",  "eq 47 47",
      "add 5 6",         "write 21 0xff80"};

  sc_time total_time = SC_ZERO_TIME;

  cout << "================ Simulation Start ================\n";

  for (auto &cmd : commands) {
    std::string *data = new std::string(cmd);

    tlm::tlm_generic_payload trans;
    trans.set_command(tlm::TLM_WRITE_COMMAND);
    trans.set_data_ptr(reinterpret_cast<unsigned char *>(data->data()));
    trans.set_data_length(data->size());

    sc_time delay = SC_ZERO_TIME;
    socket->b_transport(trans, delay);

    total_time += delay;
    delete data;
  }

  cout << "\n================ Simulation End ==================\n";
  cout << "[SW] Total simulated time: " << sc_time_stamp() << "\n";
  cout << "=================================================\n";
}
