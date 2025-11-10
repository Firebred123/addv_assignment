// tb_prog.sv
`timescale 1ns / 1ps

// Program has a port: the tb_if handle (tb_h)
program tb_prog (
    tb_if tb_h
);

  // local variables (no 'rand' qualifiers here)
  bit [15:0] rand_instr;
  int unsigned num_instructions;
  logic [7:0] mem[0:255];

  // Coverage groups (sampled on tb_h.cb)
  covergroup cg_opcode @(tb_h.cb);
    cp_opcode: coverpoint tb_h.instr[15:12] {bins all_ops[] = {[0 : 15]};}
  endgroup
  cg_opcode cg_op = new();

  covergroup cg_flags @(tb_h.cb);
    cp_flags: coverpoint tb_h.flags {bins all_flags[] = {[0 : 15]};}
  endgroup
  cg_flags cg_fl = new();

  // memory model task
  task automatic mem_model();
    forever begin
      @(tb_h.cb);
      if (tb_h.cb.mem_req) begin
        int delay = $urandom_range(0, 1);
        repeat (delay) @(tb_h.cb);
        if (tb_h.cb.mem_we) begin
          mem[tb_h.cb.mem_addr] = tb_h.cb.mem_wdata;
          tb_h.cb.mem_rdata <= 8'h00;
          tb_h.cb.mem_ready <= 1'b1;
        end else begin
          tb_h.cb.mem_rdata <= mem[tb_h.cb.mem_addr];
          tb_h.cb.mem_ready <= 1'b1;
        end
        @(tb_h.cb);
        tb_h.cb.mem_ready <= 1'b0;
        tb_h.cb.mem_rdata <= 8'h00;
      end else begin
        tb_h.cb.mem_ready <= 1'b0;
        tb_h.cb.mem_rdata <= 8'h00;
      end
    end
  endtask

  // random instruction builder
  function automatic bit [15:0] make_random_instr();
    bit [3:0] opc;
    bit [2:0] rd, rs;
    bit [3:0] imm4;
    opc  = $urandom_range(0, 15);
    rd   = $urandom_range(0, 7);
    rs   = $urandom_range(0, 7);
    imm4 = $urandom_range(0, 15);
    int r = $urandom_range(0, 99);
    if (r < 4) opc = 4'hF;
    else if (r < 12) opc = 4'h9;
    else if (r < 20) opc = 4'hA;
    return {opc, rd, 1'b0, rs, imm4};
  endfunction

  // main test sequence
  task automatic run_tests();
    wait (tb_h.rst_n == 1);
    for (int i = 0; i < 256; i++) mem[i] = $urandom_range(0, 255);
    fork
      mem_model();
    join_none

    num_instructions = 500;
    for (int i = 0; i < num_instructions; i++) begin
      bit [15:0] inst = make_random_instr();
      do @(tb_h.cb); while (!tb_h.cb.instr_ready || tb_h.cb.done);
      tb_h.cb.instr <= inst;
      tb_h.cb.instr_valid <= 1'b1;
      @(tb_h.cb);
      tb_h.cb.instr_valid <= 1'b0;
      cg_op.sample();
      cg_fl.sample();
      if (tb_h.cb.done) begin
        $display("[%0t] HALT observed after %0d instructions", $time, i + 1);
        break;
      end
    end

    repeat (10) @(tb_h.cb);
    $display("Test complete. Stopping simulation.");
    $finish;
  endtask

  // entry point called from TOP
  task start();
    tb_h.instr_valid = 0;
    tb_h.instr = 16'h0000;
    tb_h.mem_ready = 0;
    tb_h.mem_rdata = 8'h00;
    @(tb_h.cb);
    run_tests();
  endtask

endprogram
