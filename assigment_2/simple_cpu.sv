// simple_cpu.sv - JasperGold compatible version
module simple_cpu (
    input logic clk,
    input logic rst_n,

    // instruction interface
    input logic instr_valid,
    input logic [15:0] instr,
    output logic instr_ready,

    // memory interface
    input logic [7:0] mem_rdata,
    input logic mem_ready,
    output logic mem_req,
    output logic mem_we,
    output logic [7:0] mem_addr,
    output logic [7:0] mem_wdata,

    // status
    output logic done,
    output logic [3:0] flags  // {Z, N, C, V}
);

  // FSM states - using parameters instead of localparam for JasperGold
  parameter IDLE = 3'd0;
  parameter DECODE = 3'd1;
  parameter EXEC = 3'd2;
  parameter MEM = 3'd3;
  parameter WB = 3'd4;

  logic [2:0] state, next_state;
  logic [15:0] instr_reg;
  logic [7:0] regfile[0:7];
  logic [7:0] alu_result;
  logic [7:0] pc;

  // Decode fields
  logic [3:0] opcode;
  logic [2:0] rd;
  logic [2:0] rs;
  logic [3:0] imm;

  // Continuous assignments instead of always @*
  assign opcode = instr_reg[15:12];
  assign rd = instr_reg[11:9];
  assign rs = instr_reg[7:5];
  assign imm = instr_reg[3:0];

  integer i;

  // State register
  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      state <= IDLE;
    end else begin
      state <= next_state;
    end
  end

  // Next state logic - using always_comb instead of always @*
  always_comb begin
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
          next_state = MEM;
        end else if (opcode >= 4'h1 && opcode <= 4'h8) begin
          next_state = WB;
        end else begin
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
  always_ff @(posedge clk or negedge rst_n) begin
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
              pc <= pc + 8'd1;
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
              alu_result <= regfile[rd] + {4'b0, imm};
            end

            4'h7: begin  // SHL
              alu_result <= regfile[rd] << imm;
            end

            4'h8: begin  // SHR
              alu_result <= regfile[rd] >> imm;
            end

            4'h9: begin  // LOAD
              mem_addr <= regfile[rs] + {4'b0, imm};
              mem_req  <= 1'b1;
              mem_we   <= 1'b0;
            end

            4'hA: begin  // STORE
              mem_addr <= regfile[rs] + {4'b0, imm};
              mem_wdata <= regfile[rd];
              mem_req <= 1'b1;
              mem_we <= 1'b1;
            end

            4'hB: begin  // BRZ
              if (regfile[rs] == 8'h0) begin
                pc <= pc + {4'b0, imm};
              end else begin
                pc <= pc + 8'd1;
              end
            end

            4'hC: begin  // JMP
              pc <= pc + {4'b0, imm};
            end

            4'hF: begin  // HALT
              done <= 1'b1;
            end

            default: begin
              pc <= pc + 8'd1;
            end
          endcase
        end

        MEM: begin
          if (mem_ready) begin
            mem_req <= 1'b0;
            mem_we  <= 1'b0;

            if (opcode == 4'h9) begin
              regfile[rd] <= mem_rdata;
            end

            pc <= pc + 8'd1;
          end
        end

        WB: begin
          regfile[rd] <= alu_result;

          flags[3] <= (alu_result == 8'h0);
          flags[2] <= alu_result[7];
          flags[1] <= 1'b0;
          flags[0] <= 1'b0;

          pc <= pc + 8'd1;
        end

        default: begin
          // Do nothing
        end

      endcase
    end
  end

endmodule
