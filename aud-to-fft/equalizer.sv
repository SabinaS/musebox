// Frequency positions
parameter H31 = 6, H72 = 13, H150 = 28, H250 = 46, H440 = 82, H630 = 117, H1K = 186, H2_5K = 464, H5K = 929, H8K = 1486, H14K = 2601, H20k = 3715;

module equalizer (
	// Frequency components
	input  [15:0]	inreal,
	input  [15:0]	inimag,
	output [15:0]	outreal,
	output [15:0]  outimag,
	// The equalizer operates on a single clock
   input  clk,
	input  reset,
	// Request lines for the FIFOs
	output rdreq,
	output wrreq,
	// We only read when the FFT is "full".
	input  [12:0]	incount,
	// The equalizer values. Only 5 bits matter
	input  [7:0]	writedata,
	output [7:0]	readdata,
	input  [3:0]	address,
	input  chipselect,
	input  write,
	input  read
);

// Equalizer
reg read_results;

// Pipelined design
reg [12:0] pos;

// The equalizer banks
reg [4:0] b31, b72, b150, b250, b440, b630, b1k, b2_5k, b5k, b8k, b14k, b20k;

// Initialization sequence
initial begin
	pos <= 13'd0;
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
	rdreq <= 1'b0;
	wrreq <= 1'b0;
end

function [5:0] clogb2;
	input [32:0] value;
	begin
		value = value - 1;
		for (clogb2 = 0; value > 0; clogb2 = clogb2 + 1) begin
			value = value >> 1;
		end
	end
endfunction

// reg [4:0] b31, b72, b150, b250, b440, b630, b1k, b2_5k, b5k, b8k, b14k, b20k;

// Multiplication requires double the width, and adding two numbers could overflow
reg [15:0] pipe_real;
reg [15:0] pipe_imag;
reg [32:0] magn;
reg [31:0] ratio;
reg [5:0]  db;


always_ff @(posedge clk) begin
	// Wait until we have all samples
	if (!read_results && incount == SAMPLES)
		read_results <= 1'd1;
	else if (read_results) begin
		// First, compute the magnitude and ratio
		if (pos < SAMPLES) begin
			magn <= inreal * inreal + inimag * inimag;
			ratio <= inreal / inimag;
			pipe_real <= inreal;
			pipe_imag <= inimag;
		end
		// Convert the magnitude to decibles
		if (pos > 0 && pos < SAMPLES + 16'd1) begin
			db <= clogb2(magn);
			// Scale the values
			outreal <= pipe_real << 1;
			outimag <= pipe_imag << 1;
			wrreq <= 1'b1;
		end else
			wrreq <= 1'b0;
		pos <= pos + 1;
	end else begin
		rdreq <= 1'b0;
		wrreq <= 1'b0;
	end
end
// End equalizer

// Bus interface logic
always_ff @(posedge clk) begin
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
