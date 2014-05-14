parameter SAMPLES = 14'd8192;
parameter SAMPLES2 = 14'd4096;

module add_signed (
	input  signed [5:0] A,
	input  signed [5:0] B,
	output signed [6:0] sum
);

assign sum = {A[5],A} + {B[5],B} + 7'sd10;

endmodule

module audio_to_fft (
   input  aud_clk,
	input  fft_clk,
	input  system_clk,
	input  system_reset,
	input  reset,
	// If true, then we should read the data for our fft
	input  chan_req,
	input  chan_end,
   input  [15:0] audio_input,
	output [15:0] audio_output,
	output [15:0] vga_dat,
	output vga_dowrite,
	output vga_select,
	output [1:0] vga_addr,
	output [31:0] readdata,
	input  address,
	input  chipselect,
	input  read
);

// FIFO
wire wrreq;
wire rdreq;
wire [15:0] buf_cnt;
// The output of the fifo goes right to the FFT
wire [15:0] dc_out;
wire fifo_in_aclr;
dc_fifo	fifo (
	.wrclk (aud_clk),
	.wrreq (wrreq),
	.rdreq (rdreq),
	.data (audio_input),
	.rdclk (fft_clk),
	.rdusedw (buf_cnt),
	.q (dc_out),
	.aclr (fifo_in_aclr)
);
// End FIFO

// FFT for audio input
wire fft_in_can_accept_input;
wire fft_in_sisop;
wire fft_in_sieop;
wire fft_in_sivalid;
wire fft_in_sovalid;
wire fft_in_sosop;
wire fft_in_soeop;
wire [15:0] fft_in_sireal;
wire [15:0] fft_in_soreal;
wire [15:0] fft_in_soimag;
wire source_can_accept_input;
wire [5:0] fft_in_exp;
reg  [5:0] fft_in_exp_reg;
fft_module fft_in (
	.clk (fft_clk),
	.reset_n (!reset),
	.sink_error (2'b0),
	.sink_ready (fft_in_can_accept_input),
	.sink_real (fft_in_sireal),
	.sink_imag (16'b0),
	.sink_sop (fft_in_sisop),
	.sink_valid (fft_in_sivalid),
	.sink_eop (fft_in_sieop),
	// We're always ready for action
	.source_ready (source_can_accept_input),
	.source_real (fft_in_soreal),
	.source_imag (fft_in_soimag),
	.source_sop (fft_in_sosop),
	.source_eop (fft_in_soeop),
	.source_valid (fft_in_sovalid),
	.source_exp (fft_in_exp),
	.inverse (1'b0)
);

// CPU FIFO
wire cpu_rdreq;
wire cpu_wrreq;
wire [15:0] cpu_cnt;
wire cpu_is_empty;
wire [31:0] cpu_q;
cpu_fifo fft_dat (
	.rdclk (system_clk),
	.wrclk (fft_clk),
	.data ({fft_in_soreal, fft_in_soimag}),
	.wrreq (cpu_wrreq),
	.rdreq (cpu_rdreq),
	.rdusedw (cpu_cnt),
	.rdempty (cpu_is_empty),
	.q (cpu_q)
);

// FFT for equalizer to audio
wire [15:0] fft_out_sireal;
wire [15:0] fft_out_siimag;
wire fft_out_sivalid;
wire fft_out_sovalid;
wire fft_out_can_accept_input;
wire fft_out_sisop;
wire fft_out_sieop;
wire fft_out_sosop;
wire fft_out_soeop;
wire [15:0] fft_out_soreal;
wire [5:0] fft_out_exp;
reg  [5:0] fft_out_exp_reg;
fft_module fft_out (
	.clk (fft_clk),
	.reset_n (!reset),
	.sink_error (2'b0),
	.sink_ready (fft_out_can_accept_input),
	.sink_real (fft_out_sireal),
	.sink_imag (fft_out_siimag),
	.sink_sop (fft_out_sisop),
	.sink_valid (fft_out_sivalid),
	.sink_eop (fft_out_sieop),
	// We're always ready for action
	.source_ready (1'b1),
	.source_real (fft_out_soreal),
	// We can't have imaginary audio
	.source_imag (GND),
	.source_sop (fft_out_sosop),
	.source_eop (fft_out_soeop),
	.source_valid (fft_out_sovalid),
	.source_exp (fft_out_exp),
	.inverse (1'b1)
);
// End FFT

equalizer equal (
	.inreal (fft_in_soreal),
	.inimag (fft_in_soimag),
	.outreal (fft_out_sireal),
	.outimag (fft_out_siimag),
	.insop (fft_in_sosop),
	.outsop (fft_out_sisop),
	.insovalid (fft_in_sovalid),
	.outsivalid (fft_out_sivalid),
	.out_out_can_accept_input (fft_out_can_accept_input),
	.in_out_can_accept_input (source_can_accept_input),
   .clk (fft_clk)
);

// FIFO for writing to audio out
wire fifo_out_wrreq;
wire fifo_out_rdreq;
wire [15:0] fifo_out_out;
wire [15:0] fifo_out_cnt;
wire fifo_out_aclr;
dc_fifo_sm	fifo_out_real (
	.wrclk (fft_clk),
	.wrreq (fifo_out_wrreq),
	.rdreq (fifo_out_rdreq),
	.data (fft_out_soreal),
	.rdclk (aud_clk),
	.rdusedw (fifo_out_cnt),
	.q (fifo_out_out),
	.aclr (fifo_out_aclr)
);

// Shifting function
function [15:0] shift_num;
	input signed [6:0] exponent;
	input [15:0] num;
	
	begin
		case (exponent)
			-7'sd15  : shift_num = {num[15],15'b0};
			-7'sd14  : shift_num = {num[15],num[0],14'b0};
			-7'sd13  : shift_num = {num[15],num[1:0],13'b0};
			-7'sd12  : shift_num = {num[15],num[2:0],12'b0};
			-7'sd11  : shift_num = {num[15],num[3:0],11'b0};
			-7'sd10  : shift_num = {num[15],num[4:0],10'b0};
			-7'sd9   : shift_num = {num[15],num[5:0],9'b0};
			-7'sd8   : shift_num = {num[15],num[6:0],8'b0};
			-7'sd7   : shift_num = {num[15],num[7:0],7'b0};
			-7'sd6   : shift_num = {num[15],num[8:0],6'b0};
			-7'sd5   : shift_num = {num[15],num[9:0],5'b0};
			-7'sd4   : shift_num = {num[15],num[10:0],4'b0};
			-7'sd3   : shift_num = {num[15],num[11:0],3'b0};
			-7'sd2   : shift_num = {num[15],num[12:0],2'b0};
			-7'sd1   : shift_num = {num[15],num[13:0],1'b0};
			7'sd0    : shift_num = num;
			7'sd1    : shift_num = {num[15],num[15:1]};
			7'sd2    : shift_num = {num[15],num[15],num[15:2]};
			7'sd3    : shift_num = {num[15],num[15],num[15],num[15:3]};
			7'sd4    : shift_num = {num[15],num[15],num[15],num[15],num[15:4]};
			7'sd5    : shift_num = {num[15],num[15],num[15],num[15],num[15],num[15:5]};
			7'sd6    : shift_num = {num[15],num[15],num[15],num[15],num[15],num[15],num[15:6]};
			7'sd7    : shift_num = {num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15:7]};
			7'sd8    : shift_num = {num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15:8]};
			7'sd9    : shift_num = {num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15:9]};
			7'sd10   : shift_num = {num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15:10]};
			7'sd11   : shift_num = {num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15:11]};
			7'sd12   : shift_num = {num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15],num[15:12]};
			// It's too low, just zero it
			default  : shift_num = {16'b0};
		endcase
	end
endfunction

// Set the exponent
reg signed [6:0] total_exponent;
add_signed adder (
	.A (fft_out_exp_reg),
	.B (fft_in_exp_reg),
	.sum (total_exponent)
);

// Always write to the input fifo
reg in_fifo_counter;
always @(posedge aud_clk) begin
//	if (reset) begin
//		fifo_in_aclr <= 1'b1;
//		in_fifo_counter <= 1'b1;
//		wrreq <= 1'b0;
//	end else if (in_fifo_counter) begin
//		fifo_in_aclr <= 1'b0;
//		in_fifo_counter <= 1'b0;
	if (chan_end) begin
		wrreq <= 1'b1;
	end else begin
		wrreq <= 1'b0;
	end
end

// Always read from the output fifo
always @(posedge aud_clk) begin
	// The fifo is not empty
	if (fifo_out_cnt >= 12'b0 && chan_req) begin
		audio_output <= shift_num(total_exponent, fifo_out_out);
		// Acknowledge that we've read this data
		fifo_out_rdreq <= 1'b1;
	end else begin
		audio_output <= 16'b0;
		fifo_out_rdreq <= 1'b0;
	end
end

// Logic for reading from the output FFT
reg [15:0] outpos;

always @(posedge fft_clk) begin
	if (fft_out_sovalid && fft_out_soeop) begin
		outpos <= 16'd0;
		fifo_out_wrreq <= 1'b1;
		fft_out_exp_reg <= fft_out_exp;
	end else if (fft_out_sovalid) begin
		fifo_out_wrreq <= 1'b1;
		outpos <= outpos + 16'b1;
	end else if (!fft_out_sovalid) begin
		fifo_out_wrreq <= 1'b0;
	end
end

reg [15:0] pos;
reg signed [31:0] scaled;
reg signed [31:0] index = 32'sd0;
reg [15:0] fill;
reg fill_fft;

// Compute the scaled values ahead of time
always_comb begin
	scaled = dc_out;
	if (index < 32'sd2048) begin
		scaled = (scaled * index) / 32'sd2048;
	end else if (index >= 32'sd6144) begin
		scaled = (scaled * (32'sd8191 - index)) / 32'sd2048;
	end
end

// Logic for filling the input FFT
always @(posedge fft_clk) begin
	// Check to see if the fifo is "full". If it is, begin reading
	if (buf_cnt >= SAMPLES && !fill_fft) begin
		fill_fft <= 1'b1;
	end
	// Writing to the FFT
	else if (fill_fft && fft_in_can_accept_input) begin
		// Indicate to the FFT that we can send data and send the first sample
		if (pos == 16'd0) begin
			fft_in_sireal <= {scaled[31], scaled[14:0]};
			// Acknowledge that we've read this word
			rdreq <= 1'b1;
			// Assert that the data on the bus is valid
			fft_in_sivalid <= 1'b1;
			// Indicate that this is the first packet
			fft_in_sisop <= 1'b1;
			pos <= pos + 1'b1;
			index <= index + 32'sd1;
		// Deassert SOP on the next cycle
		end else if (pos == 16'd1) begin
			// Assert that the data on the bus is valid
			fft_in_sireal <= {scaled[31], scaled[14:0]};
			fft_in_sivalid <= 1'b1;
			// Acknowledge that we've read this word
			rdreq <= 1'b1;
			// Deassert SOP
			fft_in_sisop <= 1'b0;
			pos <= pos + 1'b1;
			index <= index + 32'sd1;
		// For the last sample, assert eop
		end else if (pos == SAMPLES - 15'b1) begin
			fft_in_sieop <= 1'b1;
			// Acknowledge that we've read this word
			rdreq <= 1'b1;
			// Assert that the data on the bus is valid
			fft_in_sireal <= {scaled[31], scaled[14:0]};
			fft_in_sivalid <= 1'b1;
			pos <= pos + 1'b1;
			index <= index + 32'sd1;
		// Deassert all other signals
		end else if (pos == SAMPLES) begin
			fft_in_sieop <= 1'b0;
			fft_in_sivalid <= 1'b0;
			rdreq <= 1'b0;
			pos <= 16'b0;
			index <= 32'sd0;
		// For all the other positions
		end else begin
			fft_in_sireal <= {scaled[31], scaled[14:0]};
			// Assert that the data on the bus is valid
			fft_in_sivalid <= 1'b1;
			// Acknowledge that we've read this word
			rdreq <= 1'b1;
			pos <= pos + 16'b1;
			index <= index + 32'sd1;
			vga_dowrite <= 1'b1;
			vga_select <= 1'b1;
			if (fill % 4 == 2'd3) begin
				vga_dat <= audio_input;
			end else begin
				vga_dat <= fill;
			end
			vga_addr <= fill % 4;
			fill <= fill + 16'd1;
		end
	// Don't acknowledge that we've read anything yet
	end else if (!fft_in_can_accept_input)
		rdreq <= 1'b0;
	else begin
		// Assert that the data on the bus is not valid
		fft_in_sivalid <= 1'b0;
		rdreq <= 1'b0;
		vga_dowrite <= 1'b1;
		vga_select <= 1'b1;
	end
	// Record the exponent of the fft input block
	if (fft_in_sosop && fft_in_sovalid) begin
		fft_in_exp_reg <= fft_in_exp;
		// Read this data into the cpu
		cpu_wrreq <= 1'b1;
	end else if (fft_in_sovalid) begin
		// For all other positions, always read in
		cpu_wrreq <= 1'b1;
	end else begin
		// Do nothing
		cpu_wrreq <= 1'b0;
	end
end

reg reset_cycle;

initial begin
	reset_cycle <= 1'b0;
	pos <= 16'd0;
	fill <= 16'd0;
	fill_fft <= 1'd0;
	outpos <= 16'd0;
	wrreq <= 1'b0;
	rdreq <= 1'b0;
	fifo_out_wrreq <= 1'b0;
	fifo_out_rdreq <= 1'b0;
	fft_in_sivalid <= 1'b0;
	fft_in_sisop <= 1'b0;
	fft_in_sieop <= 1'b0;
	fifo_in_aclr <= 1'b0;
	fifo_out_aclr <= 1'b0;
	in_fifo_counter <= 1'b0;
end

// Bus interface logic

always_ff @(posedge system_clk) begin
	if (system_reset) begin
		cpu_rdreq <= 1'b0;
		// I'll figure out what to do later
	end else if (chipselect && read) begin
		case (address)
			1'd0 : begin
				// In the first address, tell them our size and exponent
				readdata <= {total_exponent[6], total_exponent[6], total_exponent[6], total_exponent[6], total_exponent[6], total_exponent[6], total_exponent[6], total_exponent[6], total_exponent[6], total_exponent, cpu_cnt};
				cpu_rdreq <= 1'b0;
			end
			1'd1 : begin
				// In the second address, unconditionally give them the data
				readdata <= cpu_q;
				cpu_rdreq <= 1'b1;
			end
			default : cpu_rdreq <= 1'b0;
		endcase
	end else begin
		cpu_rdreq <= 1'b0;
	end
end

endmodule
