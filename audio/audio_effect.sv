module audio_effects (
    input  clk,
    input  sample_end,
    input  sample_req,
    output [15:0] audio_output,
    input  [15:0] audio_input,
    input  [3:0]  control,
	  // If chan is true, then the channel is the right channel. Otherwise, it is the left
	 input chan
);

reg [15:0] last_sample;
reg [15:0] dat;

assign audio_output = dat;

parameter FEEDBACK = 1;

always @(posedge clk) begin
    if (sample_end) begin
        last_sample <= audio_input;
    end

    if (sample_req) begin
        if (control[FEEDBACK] && chan) begin
            dat <= last_sample;
		  end else if (control[FEEDBACK] && !chan) begin
            dat <= last_sample;
        end else
            dat <= 16'd0;
    end
end

endmodule
