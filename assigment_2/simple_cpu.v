// simple_cpu.v - SIMPLIFIED AND FIXED
module simple_cpu (
    input clk,
    input rst_n,

    // instruction interface
    input instr_valid,
    input [15:0] instr,
    output reg instr_ready,

    // memory interface
    input [7:0] mem_rdata,
    input mem_ready,
    output reg mem_req,
    output reg mem_we,
    output reg [7:0] mem_addr,
    output reg [7:0] mem_wdata,

    // status
    output reg done,
    output reg [3:0] flags  // {Z, N, C, V}
);

  // FSM states
  localparam IDLE = 3'd0;
  localparam DECODE = 3'd1;
  localparam EXEC = 3'd2;
  localparam MEM = 3'd3;
  localparam WB = 3'd4;

  reg [2:0] state, next_state;
  reg [15:0] instr_reg;
  reg [7:0] regfile[0:7];
  reg [7:0] alu_result;
  reg [7:0] pc;

  // Decode fields
  wire [3:0] opcode = instr_reg[15:12];
  wire [2:0] rd = instr_reg[11:9];
  wire [2:0] rs = instr_reg[7:5];
  wire [3:0] imm = instr_reg[3:0];

  integer i;

  // State register
  always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      state <= IDLE;
    end else begin
      state <= next_state;
    end
  end

  // Next state logic
  always @(*) begin
    next_state = state;
    case (state)
      IDLE: begin
        if (instr_valid) next_state = DECODE;
      end

      DECODE: begin
        next_state = EXEC;
      end

      EXEC: begin
        if (opcode == 4'h9 || opcode == 4'hA) begin
          // LOAD/STORE - go to MEM
          next_state = MEM;
        end else if (opcode >= 4'h1 && opcode <= 4'h8) begin
          // ALU ops - go to WB
          next_state = WB;
        end else begin
          // NOP, Branch, Jump, HALT - back to IDLE
          next_state = IDLE;
        end
      end

      MEM: begin
        if (mem_ready) next_state = IDLE;
      end

      WB: begin
        next_state = IDLE;
      end

      default: next_state = IDLE;
    endcase
  end

  // Datapath
  always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      instr_reg <= 16'h0;
      instr_ready <= 1'b0;
      done <= 1'b0;
      mem_req <= 1'b0;
      mem_we <= 1'b0;
      mem_addr <= 8'h0;
      mem_wdata <= 8'h0;
      pc <= 8'h0;
      flags <= 4'b0;
      alu_result <= 8'h0;

      for (i = 0; i < 8; i = i + 1) begin
        regfile[i] <= 8'h0;
      end

    end else begin
      case (state)

        IDLE: begin
          instr_ready <= 1'b1;
          mem_req <= 1'b0;
          mem_we <= 1'b0;

          if (instr_valid) begin
            instr_reg   <= instr;
            instr_ready <= 1'b0;
          end
        end

        DECODE: begin
          // Just pass through
        end

        EXEC: begin
          case (opcode)
            4'h0: begin  // NOP
              pc <= pc + 1;
            end

            4'h1: begin  // ADD
              alu_result <= regfile[rd] + regfile[rs];
            end

            4'h2: begin  // SUB
              alu_result <= regfile[rd] - regfile[rs];
            end

            4'h3: begin  // AND
              alu_result <= regfile[rd] & regfile[rs];
            end

            4'h4: begin  // OR
              alu_result <= regfile[rd] | regfile[rs];
            end

            4'h5: begin  // XOR
              alu_result <= regfile[rd] ^ regfile[rs];
            end

            4'h6: begin  // ADDI
              alu_result <= regfile[rd] + imm;
            end

            4'h7: begin  // SHL
              alu_result <= regfile[rd] << imm;
            end

            4'h8: begin  // SHR
              alu_result <= regfile[rd] >> imm;
            end

            4'h9: begin  // LOAD
              mem_addr <= regfile[rs] + imm;
              mem_req  <= 1'b1;
              mem_we   <= 1'b0;
            end

            4'hA: begin  // STORE
              mem_addr <= regfile[rs] + imm;
              mem_wdata <= regfile[rd];
              mem_req <= 1'b1;
              mem_we <= 1'b1;
            end

            4'hB: begin  // BRZ
              if (regfile[rs] == 8'h0) begin
                pc <= pc + imm;
              end else begin
                pc <= pc + 1;
              end
            end

            4'hC: begin  // JMP
              pc <= pc + imm;
            end

            4'hF: begin  // HALT
              done <= 1'b1;
            end

            default: begin
              pc <= pc + 1;
            end
          endcase
        end

        MEM: begin
          if (mem_ready) begin
            mem_req <= 1'b0;
            mem_we  <= 1'b0;

            if (opcode == 4'h9) begin
              // LOAD - capture data
              regfile[rd] <= mem_rdata;
            end

            pc <= pc + 1;
          end
        end

        WB: begin
          // Write ALU result back to register
          regfile[rd] <= alu_result;

          // Update flags
          flags[3]    <= (alu_result == 8'h0);  // Zero
          flags[2]    <= alu_result[7];  // Negative
          flags[1]    <= 1'b0;  // Carry (simplified)
          flags[0]    <= 1'b0;  // Overflow (simplified)

          pc          <= pc + 1;
        end

      endcase
    end
  end

endmodule
