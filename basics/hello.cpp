// Include the SystemC library header
// This provides the definitions of sc_module, sc_main, sc_start, SC_THREAD,
// etc. All the simulation kernel and modeling constructs come from here
#include <systemc.h>

// Define a SystemC module named "Hello_SystemC"
// SC_MODULE is a macro provided by SystemC that expands into a C++ class
// It inherits from sc_module (OOP concept: Inheritance)
SC_MODULE(Hello_SystemC){

    // Constructor for the module
    // OOP concept: Constructor
    // SC_CTOR is a macro that automatically creates a constructor with the same
    // name
    // Inside it, we register processes (threads, methods, etc.)
    SC_CTOR(Hello_SystemC){// Register a thread process with the SystemC kernel
                           // OOP concept: Encapsulation (we hide implementation
                           // inside this process)
                           SC_THREAD(module_thread);
}

// Thread process definition
// This function will be executed as a separate concurrent process in simulation
// OOP concept: Member function (behavior encapsulated in the module)
void module_thread() {
  // Use SystemC's simulation-aware print (sc_time can also be printed here)
  cout << "Hello SystemC World!" << endl;
}
}
;

// Starting point of every SystemC program
// sc_main replaces the normal C++ main
// It initializes the SystemC kernel and starts the simulation
int sc_main(int argc, char *argv[]) {
  // Create an instance of our module
  // OOP concept: Object instantiation
  Hello_SystemC HelloWorld_i("HelloWorldi");

  // Start the simulation
  // The SystemC kernel will start executing all registered processes
  sc_start();

  return 0; // Normal program termination
}
