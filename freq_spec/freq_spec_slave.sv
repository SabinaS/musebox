module frec_spec_slave (
 input logic 	    clk50, reset,
 input logic [8:0]  b31, b72, b150, b250, b440, b630, b1k, b2_5k, b5k, b8k, b14k, b20k,
 output logic [7:0] VGA_R, VGA_G, VGA_B,
 output logic 	    VGA_CLK, VGA_HS, VGA_VS, VGA_BLANK_n, VGA_SYNC_n);

/*
 * 640 X 480 VGA timing for a 50 MHz clock: one pixel every other cycle
 * 
 * HCOUNT 1599 0             1279       1599 0
 *             _______________              ________
 * ___________|    Video      |____________|  Video
 * 
 * 
 * |SYNC| BP |<-- HACTIVE -->|FP|SYNC| BP |<-- HACTIVE
 *       _______________________      _____________
 * |____|       VGA_HS          |____|
 */
   // Parameters for hcount
   parameter HACTIVE      = 11'd 1280,
             HFRONT_PORCH = 11'd 32,
             HSYNC        = 11'd 192,
             HBACK_PORCH  = 11'd 96,   
             HTOTAL       = HACTIVE + HFRONT_PORCH + HSYNC + HBACK_PORCH; // 1600
   
   // Parameters for vcount
   parameter VACTIVE      = 10'd 480,
             VFRONT_PORCH = 10'd 10,
             VSYNC        = 10'd 2,
             VBACK_PORCH  = 10'd 33,
             VTOTAL       = VACTIVE + VFRONT_PORCH + VSYNC + VBACK_PORCH; // 525

   logic [10:0]			     hcount; // Horizontal counter
                                             // Hcount[10:1] indicates pixel column (0-639)
   logic 			     endOfLine;
   
   always_ff @(posedge clk50 or posedge reset)
     if (reset)          hcount <= 0;
     else if (endOfLine) hcount <= 0;
     else  	         hcount <= hcount + 11'd 1;

   assign endOfLine = hcount == HTOTAL - 1;

   // Vertical counter
   logic [9:0] 			     vcount;
   logic 			     endOfField;
   
   always_ff @(posedge clk50 or posedge reset)
     if (reset)          vcount <= 0;
     else if (endOfLine)
       if (endOfField)   vcount <= 0;
       else              vcount <= vcount + 10'd 1;

   assign endOfField = vcount == VTOTAL - 1;

   // Horizontal sync: from 0x520 to 0x5DF (0x57F)
   // 101 0010 0000 to 101 1101 1111
   assign VGA_HS = !( (hcount[10:8] == 3'b101) & !(hcount[7:5] == 3'b111));
   assign VGA_VS = !( vcount[9:1] == (VACTIVE + VFRONT_PORCH) / 2);

   assign VGA_SYNC_n = 1; // For adding sync to video signals; not used for VGA
   
   // Horizontal active: 0 to 1279     Vertical active: 0 to 479
   // 101 0000 0000  1280	       01 1110 0000  480
   // 110 0011 1111  1599	       10 0000 1100  524
   assign VGA_BLANK_n = !( hcount[10] & (hcount[9] | hcount[8]) ) &
			!( vcount[9] | (vcount[8:5] == 4'b1111) );   

   /* VGA_CLK is 25 MHz
    *             __    __    __
    * clk50    __|  |__|  |__|
    *        
    *             _____       __
    * hcount[0]__|     |_____|
    */
   assign VGA_CLK = hcount[0]; // 25 MHz clock: pixel latched on rising edge
	
	// Assign ranges for them
   parameter X31 = 10'd10, X72 = 10'd62, X150 = 10'd114, X250 = 10'd166, X440 = 10'd218, X630 = 10'd270, X1K = 10'd322, X2_5K = 10'd374, X5K = 10'd426, X8K = 10'd478, X14K = 10'd530, X20K = 10'd582;
	
	// Those offsets were computed with the width:
	parameter WIDTH = 10'd48;
	// The spacing is 4 pixels
	
	// Registers to hold the blue color
	reg [17:0] blue31, blue72, blue150, blue250, blue440, blue630, blue1k, blue2_5k, blue5k, blue8k, blue14k, blue20k;
	always_comb begin
		blue31 = 18'd255 - ({9'b0, b31} * 18'd255) / 18'd480;
		blue72 = 18'd255 - ({9'b0, b72} * 18'd255) / 18'd480;
		blue150 = 18'd255 - ({9'b0, b150} * 18'd255) / 18'd480;
		blue250 = 18'd255 - ({9'b0, b250} * 18'd255) / 18'd480;
		blue440 = 18'd255 - ({9'b0, b440} * 18'd255) / 18'd480;
		blue630 = 18'd255 - ({9'b0, b630} * 18'd255) / 18'd480;
		blue1k = 18'd255 - ({9'b0, b1k} * 18'd255) / 18'd480;
		blue2_5k = 18'd255 - ({9'b0, b2_5k} * 18'd255) / 18'd480;
		blue5k = 18'd255 - ({9'b0, b5k} * 18'd255) / 18'd480;
		blue8k = 18'd255 - ({9'b0, b8k} * 18'd255) / 18'd480;
		blue14k = 18'd255 - ({9'b0, b14k} * 18'd255) / 18'd480;
		blue20k = 18'd255 - ({9'b0, b20k} * 18'd255) / 18'd480;
	end
	
	// Color each bar
	always_comb begin
		{VGA_R, VGA_G, VGA_B} = {8'h20, 8'h20, 8'h20};
		if (hcount[10:1] >= X31 && hcount[10:1] < X31 + WIDTH) begin
			if (vcount[8:0] >= b31 && !(vcount[8:0] % 48 <= vcount[8:0] % 2)) begin
				{VGA_R, VGA_G, VGA_B} = {8'h0, 8'h0, blue31[7:0]};
			end
		end
		if (hcount[10:1] >= X72 && hcount[10:1] < X72 + WIDTH) begin
			if (vcount[8:0] >= b72 && !(vcount[8:0] % 48 <= vcount[8:0] % 2)) begin
				{VGA_R, VGA_G, VGA_B} = {8'd21, 8'h0, blue72[7:0]};
			end
		end
		if (hcount[10:1] >= X150 && hcount[10:1] < X150 + WIDTH) begin
			if (vcount[8:0] >= b150 && !(vcount[8:0] % 48 <= vcount[8:0] % 2)) begin
				{VGA_R, VGA_G, VGA_B} = {8'd43, 8'h0, blue150[7:0]};
			end
		end
		if (hcount[10:1] >= X250 && hcount[10:1] < X250 + WIDTH) begin
			if (vcount[8:0] >= b250 && !(vcount[8:0] % 48 <= vcount[8:0] % 2)) begin
				{VGA_R, VGA_G, VGA_B} = {8'd64, 8'h0, blue250[7:0]};
			end
		end
		if (hcount[10:1] >= X440 && hcount[10:1] < X440 + WIDTH) begin
			if (vcount[8:0] >= b440 && !(vcount[8:0] % 48 <= vcount[8:0] % 2)) begin
				{VGA_R, VGA_G, VGA_B} = {8'd85, 8'h0, blue440[7:0]};
			end
		end
		if (hcount[10:1] >= X630 && hcount[10:1] < X630 + WIDTH) begin
			if (vcount[8:0] >= b630 && !(vcount[8:0] % 48 <= vcount[8:0] % 2)) begin
				{VGA_R, VGA_G, VGA_B} = {8'd106, 8'h0, blue630[7:0]};
			end
		end
		if (hcount[10:1] >= X1K && hcount[10:1] < X1K + WIDTH) begin
			if (vcount[8:0] >= b1k && !(vcount[8:0] % 48 <= vcount[8:0] % 2)) begin
				{VGA_R, VGA_G, VGA_B} = {8'd128, 8'h0, blue1k[7:0]};
			end
		end
		if (hcount[10:1] >= X2_5K && hcount[10:1] < X2_5K + WIDTH) begin
			if (vcount[8:0] >= b2_5k && !(vcount[8:0] % 48 <= vcount[8:0] % 2)) begin
				{VGA_R, VGA_G, VGA_B} = {8'd149, 8'h0, blue2_5k[7:0]};
			end
		end
		if (hcount[10:1] >= X5K && hcount[10:1] < X5K + WIDTH) begin
			if (vcount[8:0] >= b5k && !(vcount[8:0] % 48 <= vcount[8:0] % 2)) begin
				{VGA_R, VGA_G, VGA_B} = {8'd170, 8'h0, blue5k[7:0]};
			end
		end
		if (hcount[10:1] >= X8K && hcount[10:1] < X8K + WIDTH) begin
			if (vcount[8:0] >= b8k && !(vcount[8:0] % 48 <= vcount[8:0] % 2)) begin
				{VGA_R, VGA_G, VGA_B} = {8'd191, 8'h0, blue8k[7:0]};
			end
		end
		if (hcount[10:1] >= X14K && hcount[10:1] < X14K + WIDTH) begin
			if (vcount[8:0] >= b14k && !(vcount[8:0] % 48 <= vcount[8:0] % 2)) begin
				{VGA_R, VGA_G, VGA_B} = {8'd212, 8'h0, blue14k[7:0]};
			end
		end
		if (hcount[10:1] >= X20K && hcount[10:1] < X20K + WIDTH) begin
			if (vcount[8:0] >= b20k && !(vcount[8:0] % 48 <= vcount[8:0] % 2)) begin
				{VGA_R, VGA_G, VGA_B} = {8'd234, 8'h0, blue20k[7:0]};
			end
		end
	end
endmodule
