module audio_effects (
    input  clk,
    input  [1:0] sample_end,
    input  [1:0] sample_req,
    output [15:0] audio_output_l,
    output [15:0] audio_output_r,
    input  [15:0] audio_input_l,
    input  [15:0] audio_input_r,
    input  [3:0]  control
	  // If chan is true, then the channel is the right channel. Otherwise, it is the left
	 // input chan
);

reg [15:0] dat_l;
reg [15:0] dat_r;
reg [15:0] last_sample_l;
reg [15:0] last_sample_r;

initial begin
	dat_l <= 16'd0;
	dat_r <= 16'd0;
	last_sample_l <= 16'd0;
	last_sample_r <= 16'd0;
end

assign audio_output_l = dat_l;
assign audio_output_r = dat_r;

parameter FEEDBACK = 1;

always @(posedge clk) begin
	if (sample_end[1]) begin
		last_sample_l <= audio_input_l;
	end else if (sample_end[0]) begin
		last_sample_r <= audio_input_r;
	end
	
	if (sample_req[1]) begin
		if (control[FEEDBACK]) begin
			dat_l <= last_sample_l;
		end else begin
			dat_l <= 16'd0;
		end
	end else if (sample_req[0]) begin
		if (control[FEEDBACK]) begin
			dat_r <= last_sample_r;
		end else begin
			dat_r <= 16'd0;
		end
	end
end

endmodule
