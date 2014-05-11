parameter SAMPLES = 13'd4096;

module audio_to_fft (
   input  aud_clk,
	input  fft_clk,
	input  reset,
	// If true, then we should read the data for our fft
	input  chan_req,
	input  chan_end,
   input  [15:0] audio_input,
	output [15:0] audio_output,
	output [15:0] vga_dat,
	output vga_dowrite,
	output vga_select,
	output [1:0] vga_addr
);

// FIFO
wire wrreq;
wire rdreq;
wire [12:0] buf_cnt;
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
wire [15:0] fft_in_soreal;
wire [15:0] fft_in_soimag;
wire fft_out_can_accept_input;
wire [5:0] fft_in_exp;
reg  [5:0] fft_in_exp_reg;

fft_module fft_in (
	.clk (fft_clk),
	.reset_n (!reset),
	.sink_error (2'b0),
	.sink_ready (fft_in_can_accept_input),
	.sink_real (dc_out),
	.sink_imag (16'b0),
	.sink_sop (fft_in_sisop),
	.sink_valid (fft_in_sivalid),
	.sink_eop (fft_in_sieop),
	// We're always ready for action
	.source_ready (fft_out_can_accept_input),
	.source_real (fft_in_soreal),
	.source_imag (fft_in_soimag),
	.source_sop (fft_in_sosop),
	.source_eop (fft_in_soeop),
	.source_valid (fft_in_sovalid),
	.source_exp (fft_in_exp),
	.inverse (1'b0)
);

// FFT for equalizer to audio
wire fft_out_sovalid;
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
	.sink_real (fft_in_soreal),
	.sink_imag (fft_in_soimag),
	.sink_sop (fft_in_sosop),
	.sink_valid (fft_in_sovalid),
	.sink_eop (fft_in_soeop),
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

// FIFO for writing to audio out
wire fifo_out_wrreq;
wire fifo_out_rdreq;
wire [15:0] fifo_out_out;
wire [11:0] fifo_out_cnt;
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
		audio_output <= fifo_out_out >> 12;
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
reg [15:0] fill;
reg fill_fft;

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
			// Acknowledge that we've read this word
			rdreq <= 1'b1;
			// Assert that the data on the bus is valid
			fft_in_sivalid <= 1'b1;
			// Indicate that this is the first packet
			fft_in_sisop <= 1'b1;
			pos <= pos + 1'b1;
		// Deassert SOP on the next cycle
		end else if (pos == 16'd1) begin
			// Assert that the data on the bus is valid
			fft_in_sivalid <= 1'b1;
			// Acknowledge that we've read this word
			rdreq <= 1'b1;
			// Deassert SOP
			fft_in_sisop <= 1'b0;
			pos <= pos + 1'b1;
		// For the last sample, assert eop
		end else if (pos == SAMPLES - 15'b1) begin
			fft_in_sieop <= 1'b1;
			// Acknowledge that we've read this word
			rdreq <= 1'b1;
			// Assert that the data on the bus is valid
			fft_in_sivalid <= 1'b1;
			pos <= pos + 1'b1;
		// Deassert all other signals
		end else if (pos == SAMPLES) begin
			fft_in_sieop <= 1'b0;
			fft_in_sivalid <= 1'b0;
			rdreq <= 1'b0;
			pos <= 16'b0;
		// For all the other positions
		end else begin
			// Assert that the data on the bus is valid
			fft_in_sivalid <= 1'b1;
			// Acknowledge that we've read this word
			rdreq <= 1'b1;
			pos <= pos + 16'b1;
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
	if (fft_in_sosop) begin
		fft_in_exp_reg <= fft_in_exp;
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

endmodule
