#include <systemc>
using namespace sc_core;

SC_MODULE(Hello){SC_CTOR(Hello){SC_THREAD(run);
}
void run() { std::cout << "Hello SystemC at " << sc_time_stamp() << std::endl; }
}
;

int sc_main(int argc, char *argv[]) {
  Hello h("h");
  sc_start(1, SC_NS);
  return 0;
}
