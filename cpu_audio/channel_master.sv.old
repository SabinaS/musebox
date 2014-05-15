parameter TRANSFORM_LENGTH = 16'd32768;

module channel_master (
	input  aud_clk,
	input  aud_reset,
	input  system_clk,
	input  system_reset,
	// If true, then our channel is active
	input  [1:0] chan_req,
	input  [1:0] chan_end,
	output [15:0] audio_input_l,
	output [15:0] audio_input_r,
	output [15:0] audio_output_l,
	output [15:0] audio_output_r,
	output [15:0] vga_dat,
	output vga_dowrite,
	output vga_select,
	output [1:0] vga_addr,
	output [31:0] readdata,
	input  [31:0] writedata,
	input  address,
	input  chipselect,
	input  read,
	input  write
);

assign audio_output_l = audio_input_l;
assign audio_output_r = audio_input_r;

// FIFOs going from audio to CPU
wire li_rdreq;
wire ri_rdreq;
wire li_wrreq;
wire ri_wrreq;
wire [15:0] li_q;
wire [15:0] ri_q;
wire [15:0] li_dat;
wire [15:0] ri_dat;
wire [15:0] li_rdw;
wire [15:0] ri_rdw;
dcfifo_atc li (
	.rdclk (system_clk),
	.wrclk (aud_clk),
	.wrreq (li_wrreq),
	.rdreq (li_rdreq),
	.data (audio_input_l),
	.q (li_q),
	.rdusedw (li_rdw)
);

dcfifo_atc ri (
	.rdclk (system_clk),
	.wrclk (aud_clk),
	.wrreq (ri_wrreq),
	.rdreq (ri_rdreq),
	.data (audio_input_r),
	.q (ri_q),
	.rdusedw (ri_rdw)
);

// FIFOs going from CPU to audio
wire lo_wrreq;
wire lo_rdreq;
wire [15:0] lo_dat;
wire [15:0] lo_q;
wire [14:0] lo_rdw;
wire ro_wrreq;
wire ro_rdreq;
wire [15:0] ro_dat;
wire [15:0] ro_q;
wire [14:0] ro_rdw;
dcfifo_cta lo (
	.rdclk (aud_clk),
	.wrclk (system_clk),
	.wrreq (lo_wrreq),
	.rdreq (lo_rdreq),
	.data (lo_dat),
	.q (lo_q),
	.rdusedw (lo_rdw)
);
dcfifo_cta ro (
	.rdclk (aud_clk),
	.wrclk (system_clk),
	.wrreq (ro_wrreq),
	.rdreq (ro_rdreq),
	.data (ro_dat),
	.q (ro_q),
	.rdusedw (ro_rdw)
);

// Sound input logic (sound always comes in in pairs)
always@(posedge aud_clk)begin
	if (chan_end[1]) begin
		li_wrreq <= 1'b1;
		ri_wrreq <= 1'b0;
	end else if (chan_end[0]) begin
		ri_wrreq <= 1'b1;
		li_wrreq <= 1'b0;
	end else begin
		li_wrreq <= 1'b0;
		ri_wrreq <= 1'b0;
	end
end

// Sound output logic
reg [1:0] valid_pair = 2'b0;
always@(posedge aud_clk) begin
//	lo_rdreq <= 1'b0;
//	ro_rdreq <= 1'b0;
//	audio_output_l <= 16'b0;
//	audio_output_r <= 16'b0;
	if (chan_end[1] && li_rdw > 16'b0) begin
//		audio_output_l <= li_q;
//		audio_output_r <= 16'b0;
		li_rdreq <= 1'b1;
		ri_rdreq <= 1'b0;
	end else if (chan_end[0] && ri_rdw > 16'd0) begin
//		audio_output_r <= ri_q;
//		audio_output_l <= 16'b0;
		ri_rdreq <= 1'b1;
		li_rdreq <= 1'b0;
	end else begin
		ri_rdreq <= 1'b0;
		li_rdreq <= 1'b0;
	end
//	if (lo_rdw > 15'd0 && ro_rdw > 15'd0 && valid_pair == 2'b0) begin
//		valid_pair <= 2'd2;
//	// If we have data to read, it was requested, and the other channel has data, let's go
//	end if (lo_rdw > 15'd0 && chan_req[1] && valid_pair > 2'd0) begin
//		valid_pair <= valid_pair - 2'd1;
//		audio_output_l <= lo_q;
//		lo_rdreq <= 1'b1;
//	end if (ro_rdw > 15'd0 && chan_req[0] && valid_pair > 2'd0) begin
//		valid_pair <= valid_pair - 2'd1;
//		audio_output_r <= ro_q;
//		ro_rdreq <= 1'b1;
//	end
end

//// Bus interface logic
//always@(posedge system_clk) begin
//	// Initially set all requests to 0
//	li_rdreq <= 1'b0;
//	ri_rdreq <= 1'b0;
//	lo_wrreq <= 1'b0;
//	ro_wrreq <= 1'b0;
//	if (system_reset) begin
//		// To do
//	// Writing to the output FIFOs
//	end else if (chipselect && write) begin
//		case (address)
//			1'b0 : begin
//				lo_wrreq <= 1'b1;
//				lo_dat <= writedata[31:16];
//				ro_wrreq <= 1'b1;
//				ro_dat <= writedata[15:0];
//			end
//		endcase
//	// Reading from the input FIFOs
//	end else if (chipselect && read) begin
//		case (address)
//			1'b0 : begin
//				readdata <= {li_rdw, ri_rdw};
//			end
//			1'b1 : begin
//				readdata <= {li_q, ri_q};
//				li_rdreq <= 1'b1;
//				ri_rdreq <= 1'b1;
//			end
//		endcase
//	end
//end

endmodule
