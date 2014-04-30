module audio_to_fft (
   input  aud_clk,
	input  fft_clk,
	input  reset,
	// If chan is true, then the channel is the right channel. Otherwise, it is the left
	input  chan,
   input  [15:0] audio_input,
	output [15:0] audio_output
);

parameter SAMPLES = 16'd32768;

// FIFO
wire lwrite;
wire rwrite;
wire [15:0] rcount;
wire [15:0] lcount;
wire [15:0] rq;
wire [15:0] lq;
// For the left channel
dc_fifo	fifo_left (
	.wrclk (aud_clk),
	.wrreq (lwrite),
	.data (audio_input),
	.rdclk (fft_clk),
	.rdusedw (rcount),
	.q (lq),
	.aclr (1'b0)
);
// For the right channel
dc_fifo	fifo_right (
	.wrclk (aud_clk),
	.wrreq (rwrite),
	.data (audio_input),
	.rdclk (fft_clk),
	.rdusedw (lcount),
	.q (rq),
	.aclr (1'b0)
);
// End FIFOs

// FFT for audio to cpu
wire fft_in_rst = reset;
wire fft_in_can_accept_input;
wire fft_in_sidat;
wire fft_in_sisop;
wire fft_in_sieop;
wire fft_in_sivalid;

fft_module fft_in (
	.clk (fft_clk),
	.reset_n (fft_in_rst),
	.sink_error (2'b0),
	.sink_ready (fft_in_can_accept_input),
	.sink_imag (16'b0),
	.sink_real (fft_in_sidat),
	.sink_sop (fft_in_sisop),
	.sink_valid (fft_in_sivalid),
	.sink_eop (fft_in_sieop)
);
// End FFT

// fifo_reading[1] == 1 means that the right channel is reading,
// fifo_reading[0] == 1 means the left channel is reading
wire [1:0] fifo_reading;

initial begin
	rwrite = 1'b0;
	lwrite = 1'b0;
	fifo_reading = 2'b0;
end

always @(posedge aud_clk) begin
	// Always write to the fifo
	if (chan) begin
		rwrite <= 1'b1;
		lwrite <= 1'b0;
	end else begin
		lwrite <= 1'b1;
		rwrite <= 1'b0;
	end
end

reg [14:0] index = 0;

always @(posedge fft_clk) begin
	// Check to see if the fifo is "full". If it is, begin reading
	if (rcount >= SAMPLES && fifo_reading == 2'b0) begin
		fifo_reading[1] <= 1'b1;
	end else if (lcount >= SAMPLES && fifo_reading == 2'b0) begin
		fifo_reading[0] <= 1'b1;
	end
	// Writing to the FFT
	if (fifo_reading == 2'b01 || fifo_reading == 2'b10) begin
		// Initial parameters
		if (index == 15'b0) begin
			fft_in_sivalid <= 1;
			fft_in_sisop <= 1;
		end
	end
end

endmodule
