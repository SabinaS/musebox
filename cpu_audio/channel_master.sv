module channel_master (
	// Purely audio
   input  aud_clk,
   input  [1:0] sample_end,
   input  [1:0] sample_req,
   output [15:0] audio_output_l,
   output [15:0] audio_output_r,
   input  [15:0] audio_input_l,
   input  [15:0] audio_input_r,
   input  [3:0]  control,
	// System bus stuff
	output [31:0] readdata,
	input  [31:0] writedata,
	input  address,
	input  chipselect,
	input  read,
	input  write,
	input  system_clk,
	input  system_reset
);

reg [15:0] dat_l;
reg [15:0] dat_r;
reg [15:0] last_sample_l;
reg [15:0] last_sample_r;

// Audio Buffer. These two fifos buffer audio and send it over to the CPU
wire li_rdreq;
wire ri_rdreq;
wire li_wrreq;
wire ri_wrreq;
wire [15:0] li_q;
wire [15:0] ri_q;
wire [15:0] li_rdw;
wire [15:0] ri_rdw;
wire li_wrfull;
wire ri_wrfull;
dcfifo_atc li (
	.rdclk (system_clk),
	.wrclk (aud_clk),
	.wrreq (li_wrreq),
	.rdreq (li_rdreq),
	.data (audio_input_l),
	.q (li_q),
	.rdusedw (li_rdw),
	.wrfull (li_wrfull)
);

dcfifo_atc ri (
	.rdclk (system_clk),
	.wrclk (aud_clk),
	.wrreq (ri_wrreq),
	.rdreq (ri_rdreq),
	.data (audio_input_r),
	.q (ri_q),
	.rdusedw (ri_rdw),
	.wrfull (ri_wrfull)
);

// FIFOs going from CPU to audio. These fifos buffer the audio the CPU sends
wire lo_wrreq;
wire lo_rdreq;
wire [15:0] lo_dat;
wire [15:0] lo_q;
wire [14:0] lo_rdw;
wire lo_rdempty;
wire lo_wrfull;
wire ro_wrreq;
wire ro_rdreq;
wire [15:0] ro_dat;
wire [15:0] ro_q;
wire [14:0] ro_rdw;
wire ro_rdempty;
wire ro_wrfull;
dcfifo_cta lo (
	.rdclk (aud_clk),
	.wrclk (system_clk),
	.wrreq (lo_wrreq),
	.rdreq (lo_rdreq),
	.data (lo_dat),
	.q (lo_q),
	.rdusedw (lo_rdw),
	.rdempty (lo_rdempty),
	.wrfull (lo_wrfull)
);
dcfifo_cta ro (
	.rdclk (aud_clk),
	.wrclk (system_clk),
	.wrreq (ro_wrreq),
	.rdreq (ro_rdreq),
	.data (ro_dat),
	.q (ro_q),
	.rdusedw (ro_rdw),
	.rdempty (ro_rdempty),
	.wrfull (ro_wrfull)
);

initial begin
	dat_l <= 16'd0;
	dat_r <= 16'd0;
	last_sample_l <= 16'd0;
	last_sample_r <= 16'd0;
end

assign audio_output_l = dat_l;
assign audio_output_r = dat_r;

parameter FEEDBACK = 1;
parameter CPU = 3;

always @(posedge aud_clk) begin
	// In every case, make sure to specify a state of the FIFO reads and writes
	if (sample_end[1]) begin
		last_sample_l <= audio_input_l;
		// Don't write if full
		if (li_wrfull || ri_wrfull) begin
			li_wrreq <= 1'b0;
		end else begin
			li_wrreq <= 1'b1;
		end
		ri_wrreq <= 1'b0;
		// li and ri wrreq determined for this branch
	end else if (sample_end[0]) begin
		last_sample_r <= audio_input_r;
		li_wrreq <= 1'b0;
		// Don't write if full
		if (ri_wrfull || li_wrfull) begin
			ri_wrreq <= 1'b0;
		end else begin
			ri_wrreq <= 1'b1;
		end
		// And for this one
	end else begin
		li_wrreq <= 1'b0;
		ri_wrreq <= 1'b0;
		// And for this one
	end
	
	// Reading works a bit differently. Always retrieve the next sample from the
	// output FIFOs, even if you don't use it.
	if (sample_req[1]) begin
		// This determines lo_rdreq for this branch
		if (lo_rdempty) begin
			lo_rdreq <= 1'b0;
		end else begin
			lo_rdreq <= 1'b1;
		end
		// And ro_rdreq
		ro_rdreq <= 1'b0;
		if (control[CPU]) begin
			// Mute if empty
			if (lo_rdempty) begin
				dat_l <= 16'b0;
			end else begin
				dat_l <= lo_q;
			end
		end else if (control[FEEDBACK]) begin
			dat_l <= last_sample_l;
		end else begin
			dat_l <= 16'd0;
		end
	end else if (sample_req[0]) begin
		// This determines ro_rdreq for this branch
		if (ro_rdempty) begin
			ro_rdreq <= 1'b0;
		end else begin
			ro_rdreq <= 1'b1;
		end
		// And lo_rdreq
		lo_rdreq <= 1'b0;
		if (control[CPU]) begin
			// Mute if empty
			if (ro_rdempty) begin
				dat_r <= 16'b0;
			end else begin
				dat_r <= ro_q;
			end
		end else if (control[FEEDBACK]) begin
			dat_r <= last_sample_r;
		end else begin
			dat_r <= 16'd0;
		end
	end else begin
		// This determines lo and ro rdreq for this branch
		lo_rdreq <= 1'b0;
		ro_rdreq <= 1'b0;
	end
end

// Bus interface logic
always@(posedge system_clk) begin
	// Initially set all requests to 0
	if (system_reset) begin
		li_rdreq <= 1'b0;
		ri_rdreq <= 1'b0;
		lo_wrreq <= 1'b0;
		ro_wrreq <= 1'b0;
	// Writing to the output FIFOs
	end else if (chipselect && write) begin
		case (address)
			1'b0 : begin
				lo_wrreq <= 1'b1;
				lo_dat <= writedata[31:16];
				ro_wrreq <= 1'b1;
				ro_dat <= writedata[15:0];
				li_rdreq <= 1'b0;
				ri_rdreq <= 1'b0;
			end
			default : begin
				li_rdreq <= 1'b0;
				ri_rdreq <= 1'b0;
				lo_wrreq <= 1'b0;
				ro_wrreq <= 1'b0;
			end
		endcase
	// Reading from the input FIFOs
	end else if (chipselect && read) begin
		case (address)
			1'b0 : begin
				readdata <= {li_rdw, ri_rdw};
				li_rdreq <= 1'b0;
				ri_rdreq <= 1'b0;
				lo_wrreq <= 1'b0;
				ro_wrreq <= 1'b0;
			end
			1'b1 : begin
				readdata <= {li_q, ri_q};
				li_rdreq <= 1'b1;
				ri_rdreq <= 1'b1;
				lo_wrreq <= 1'b0;
				ro_wrreq <= 1'b0;
			end
			default : begin
				li_rdreq <= 1'b0;
				ri_rdreq <= 1'b0;
				lo_wrreq <= 1'b0;
				ro_wrreq <= 1'b0;
			end
		endcase
	end else begin
		li_rdreq <= 1'b0;
		ri_rdreq <= 1'b0;
		lo_wrreq <= 1'b0;
		ro_wrreq <= 1'b0;
	end
end

endmodule
