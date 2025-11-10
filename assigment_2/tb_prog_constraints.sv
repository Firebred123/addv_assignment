// Constrained program-style testbench for simple_cpu
// Instantiate in TOP as: tb_prog tb_prog_inst (tb_if_i);
`timescale 1ns / 1ps

program tb_prog_c (
    tb_if tb_h
);

  // Local state / variables
  bit [15:0] rand_instr;
  int unsigned num_instructions;
  logic [7:0] mem[0:255];
  int instr_count;
  int seed;

  // ---------------------------------------------------------------------
  // Combined covergroup with opcode coverpoint, flags coverpoint and cross
  // Automatically sampled on tb_h.cb (posedge clock)
  // ---------------------------------------------------------------------
  covergroup cg_opfl @(tb_h.cb);
    // opcode coverpoint
    cp_opcode: coverpoint tb_h.instr[15:12] {
      bins nop   = {4'h0};
      bins add   = {4'h1};
      bins sub   = {4'h2};
      bins and_  = {4'h3};
      bins or_   = {4'h4};
      bins xor_  = {4'h5};
      bins addi  = {4'h6};
      bins shl   = {4'h7};
      bins shr   = {4'h8};
      bins load  = {4'h9};
      bins store = {4'hA};
      bins brz   = {4'hB};
      bins jmp   = {4'hC};
      bins halt  = {4'hD};
      bins others = default;
    }
  endgroup

  cg_opfl cg_inst = new();

  // ---------------------------------------------------------------------
  // Constraint section for generating valid instruction patterns
  // ---------------------------------------------------------------------
  constraint opcode_c {
    // Include all instruction opcodes for full coverage
    rand_instr[15:12] inside {
      4'h0, // nop
      4'h1, // add
      4'h2, // sub
      4'h3, // and
      4'h4, // or
      4'h5, // xor
      4'h6, // addi
      4'h7, // shl
      4'h8, // shr
      4'h9, // load
      4'hA, // store
      4'hB, // brz
      4'hC, // jmp
      4'hD  // halt
    };
  }

  // Balanced weighting so rare ops get tested too
  constraint opcode_weight_c {
    rand_instr[15:12] dist {
      4'h0 := 2, // nop
      4'h1 := 2, // add
      4'h2 := 2, // sub
      4'h3 := 2, // and
      4'h4 := 2, // or
      4'h5 := 2, // xor
      4'h6 := 2, // addi
      4'h7 := 2, // shl
      4'h8 := 2, // shr
      4'h9 := 5, // load
      4'hA := 5, // store
      4'hB := 5, // brz
      4'hC := 5, // jmp
      4'hD := 3  // halt
    };
  }

  // Memory access validity
  constraint mem_addr_c {
    rand_instr[7:0] < 8'd128; // restrict address range to lower memory
  }

  // ---------------------------------------------------------------------
  // Initial block: random instruction generation and driving
  // ---------------------------------------------------------------------
  initial begin
    num_instructions = 200;
    seed = 42;
    instr_count = 0;
    std::randomize(seed);

    for (int i = 0; i < num_instructions; i++) begin
      assert(std::randomize(rand_instr))
        else $fatal("Randomization failed at %0d", i);

      tb_h.instr = rand_instr;
      tb_h.instr_valid = 1'b1;

      @(tb_h.cb);
      tb_h.instr_valid = 1'b0;

      // Sample covergroup
      cg_inst.sample();

      instr_count++;
    end

    $display("Program generation completed: %0d instructions", instr_count);
  end

endprogram
