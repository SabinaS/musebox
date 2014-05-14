module frec_spec(input logic        clk,
	       input logic 	  reset,
	       input logic [15:0]  writedata,
	       input logic 	  write,
	       input 		  chipselect,
	       input logic [3:0]  address,

	       output logic [7:0] VGA_R, VGA_G, VGA_B,
	       output logic 	  VGA_CLK, VGA_HS, VGA_VS, VGA_BLANK_n,
	       output logic 	  VGA_SYNC_n);

	// Keep a bank of heights for the bars
	reg [8:0] b31 = 9'd240, b72 = 9'd240, b150 = 9'd240, b250 = 9'd240, b440 = 9'd240, b630 = 9'd240, b1k = 9'd240, b2_5k = 9'd240, b5k = 9'd240, b8k = 9'd240, b14k = 9'd240, b20k = 9'd240;

   frec_spec_slave slave(.clk50(clk), .*);

	always_ff @(posedge clk)
		if (reset) begin
			b31 	<= 9'd240;
			b72 	<= 9'd240;
			b150 	<= 9'd240;
			b250 	<= 9'd240;
			b440 	<= 9'd240;
			b630 	<= 9'd240;
			b1k 	<= 9'd240;
			b2_5k <= 9'd240;
			b5k 	<= 9'd240;
			b8k 	<= 9'd240;
			b14k 	<= 9'd240;
			b20k 	<= 9'd240;
		end else if (chipselect && write)
			case (address)
				4'd0	:	b31 	<= writedata[8:0];
				4'd1	:	b72 	<= writedata[8:0];
				4'd2	:	b150 	<= writedata[8:0];
				4'd3	:	b250 	<= writedata[8:0];
				4'd4	:	b440 	<= writedata[8:0];
				4'd5	:	b630 	<= writedata[8:0];
				4'd6	:	b1k 	<= writedata[8:0];
				4'd7	:	b2_5k <= writedata[8:0];
				4'd8	:	b5k 	<= writedata[8:0];
				4'd9	:	b8k 	<= writedata[8:0];
				4'd10	:	b14k 	<= writedata[8:0];
				4'd11	:	b20k 	<= writedata[8:0];
			endcase
	       
endmodule
