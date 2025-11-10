//========================================
// Testbench Program Block
//========================================
program tb_prog_c (
    tb_if tb_h  // interface handle to DUT
);

  // ------------------------------------
  // Local variables
  // ------------------------------------
  bit [15:0] rand_instr;
  int unsigned num_instructions;
  logic [7:0] mem[0:255];
  int instr_count;
  int seed;

  // ------------------------------------
  // Class handle
  // ------------------------------------
  instr_gen_c gen;

  // ------------------------------------
  // Initial block
  // ------------------------------------
  initial begin
    // setup
    seed = 12345;
    num_instructions = 50;
    instr_count = 0;

    // construct generator
    gen = new(seed);

    $display("[TB_PROG] Starting random instruction generation...");

    // generate & load random instructions
    repeat (num_instructions) begin
      rand_instr         = gen.random_instr();
      mem[instr_count]   = rand_instr[7:0];
      mem[instr_count+1] = rand_instr[15:8];
      tb_h.instr_in      = rand_instr;
      tb_h.instr_valid   = 1'b1;

      @(posedge tb_h.clk);
      tb_h.instr_valid = 1'b0;

      instr_count += 2;
      @(posedge tb_h.clk);
    end

    $display("[TB_PROG] Finished loading %0d instructions.", num_instructions);
    $finish;
  end

endprogram : tb_prog_c


//========================================
// Example instruction generator class
//========================================
class instr_gen_c;
  rand bit [15:0] instr;
  int seed;

  function new(int seed_in = 0);
    seed = seed_in;
    void'($urandom(seed));
  endfunction

  function bit [15:0] random_instr();
    // Simple random pattern (customize as per ISA)
    instr = $urandom_range(0, 16'hFFFF);
    return instr;
  endfunction
endclass
