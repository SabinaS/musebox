// Frequency positions
parameter H31 = 14'd6, H72 = 14'd13, H150 = 14'd28, H250 = 14'd46, H440 = 14'd82, H630 = 14'd117, H1K = 14'd186, H2_5K = 14'd464, H5K = 14'd929, H8K = 14'd1486, H14K = 14'd2601, H20K = 14'd3715;

module equalizer (
	// Frequency components
	input  [15:0]	inreal,
	input  [15:0]	inimag,
	output [15:0]	outreal,
	output [15:0]  outimag,
	input insop,
	output outsop,
	input ineop,
	output outeop,
	input insovalid,
	output outsivalid,
	input out_out_can_accept_input,
	output in_out_can_accept_input,
	// The equalizer operates on a single clock
   input  clk,
	input  system_clk,
	input  reset,
	// The equalizer values. Only 5 bits matter
	input  [7:0]	writedata,
	output [7:0]	readdata,
	input  [3:0]	address,
	input  chipselect,
	input  write,
	input  read
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

// The equalizer banks
reg [4:0] b31, b72, b150, b250, b440, b630, b1k, b2_5k, b5k, b8k, b14k, b20k;
reg signed [6:0] s31, s72, s150, s250, s440, s630, s1k, s2_5k, s5k, s8k, s14k, s20k;

// Signed assigner
always_comb begin
	s31 = b31;
	s31 = 7'sd13 - s31;
	s72 = b72;
	s72 = 7'sd13 - s72;
	s150 = b150;
	s150 = 7'sd13 - s150;
	s250 = b250;
	s250 = 7'sd13 - s250;
	s440 = b440;
	s440 = 7'sd13 - s440;
	s630 = b630;
	s630 = 7'sd13 - s630;
	s1k = b1k;
	s1k = 7'sd13 - s1k;
	s2_5k = b2_5k;
	s2_5k = 7'sd13 - s2_5k;
	s5k = b5k;
	s5k = 7'sd13 - s5k;
	s8k = b8k;
	s8k = 7'sd13 - s8k;
	s14k = b14k;
	s14k = 7'sd13 - s14k;
	s20k = b20k;
	s20k = 7'sd13 - s20k;
end

// Equalizer
reg [13:0] pos = 14'b0;
always_comb begin
	outsop = insop;
	outeop = ineop;
	outsivalid = insovalid;
	in_out_can_accept_input = out_out_can_accept_input;
	outimag = inimag;
	outreal = inreal;
	if (insop) begin
		pos = 14'b0;
	end else begin
		pos = pos + 14'b1;
	end
	// Scale by the ranges
	if (pos < H31 + 14'd4) begin
		outimag = shift_num(s31, inimag);
		outreal = shift_num(s31, inreal);
	end else if (pos < H72 + 14'd7) begin
		outimag = shift_num(s72, inimag);
		outreal = shift_num(s72, inreal);
	end else if (pos < H150 + 14'd9) begin
		outimag = shift_num(s150, inimag);
		outreal = shift_num(s150, inreal);
	end else if (pos < H250 + 14'd18) begin
		outimag = shift_num(s250, inimag);
		outreal = shift_num(s250, inreal);
	end else if (pos < H440 + 14'd18) begin
		outimag = shift_num(s440, inimag);
		outreal = shift_num(s440, inreal);
	end else if (pos < H630 + 14'd34) begin
		outimag = shift_num(s630, inimag);
		outreal = shift_num(s630, inreal);
	end else if (pos < H1K + 14'd139) begin
		outimag = shift_num(s1k, inimag);
		outreal = shift_num(s1k, inreal);
	end else if (pos < H2_5K + 14'd232) begin
		outimag = shift_num(s2_5k, inimag);
		outreal = shift_num(s2_5k, inreal);
	end else if (pos < H5K + 14'd279) begin
		outimag = shift_num(s72, inimag);
		outreal = shift_num(s72, inreal);
	end else if (pos < H8K + 14'd557) begin
		outimag = shift_num(s8k, inimag);
		outreal = shift_num(s8k, inreal);
	end else if (pos < H14K + 14'd557) begin
		outimag = shift_num(s14k, inimag);
		outreal = shift_num(s14k, inreal);
	end else if (pos < H20K + 14'd381) begin
		outimag = shift_num(s20k, inimag);
		outreal = shift_num(s20k, inreal);
	end
end
reg read_results;

// Initialization sequence
initial begin
	b31 <= 5'd13;
	b72 <= 5'd13;
	b150 <= 5'd13;
	b250 <= 5'd13;
	b440 <= 5'd13;
	b630 <= 5'd13;
	b1k <= 5'd13;
	b2_5k <= 5'd13;
	b5k <= 5'd13;
	b8k <= 5'd13;
	b14k <= 5'd13;
	b20k <= 5'd13;
	readdata <= 8'd0;
end
// End equalizer

// Bus interface logic
always_ff @(posedge system_clk) begin
	if (reset) begin
		b31 <= 5'd13;
		b72 <= 5'd13;
		b150 <= 5'd13;
		b250 <= 5'd13;
		b440 <= 5'd13;
		b630 <= 5'd13;
		b1k <= 5'd13;
		b2_5k <= 5'd13;
		b5k <= 5'd13;
		b8k <= 5'd13;
		b14k <= 5'd13;
		b20k <= 5'd13;
		readdata <= 8'd0;
	end else if (chipselect && write) begin
		case (address)
			4'd0 : b31 <= writedata[4:0];
			4'd1 : b72 <= writedata[4:0];
			4'd2 : b150 <= writedata[4:0];
			4'd3 : b250 <= writedata[4:0];
			4'd4 : b440 <= writedata[4:0];
			4'd5 : b630 <= writedata[4:0];
			4'd6 : b1k <= writedata[4:0];
			4'd7 : b2_5k <= writedata[4:0];
			4'd8 : b5k <= writedata[4:0];
			4'd9 : b8k <= writedata[4:0];
			4'd10 : b14k <= writedata[4:0];
			4'd11 : b20k <= writedata[4:0];
		endcase
	end else if (chipselect && read) begin
		case (address)
			4'd0 : readdata <= {3'd0, b31};
			4'd1 : readdata <= {3'd0, b72};
			4'd2 : readdata <= {3'd0, b150};
			4'd3 : readdata <= {3'd0, b250};
			4'd4 : readdata <= {3'd0, b440};
			4'd5 : readdata <= {3'd0, b630};
			4'd6 : readdata <= {3'd0, b1k};
			4'd7 : readdata <= {3'd0, b2_5k};
			4'd8 : readdata <= {3'd0, b5k};
			4'd9 : readdata <= {3'd0, b8k};
			4'd10 : readdata <= {3'd0, b14k};
			4'd11 : readdata <= {3'd0, b20k};
		endcase
	end
end

endmodule
