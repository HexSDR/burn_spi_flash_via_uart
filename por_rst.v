
/*
   por_rst #(.CNTR_END(1000*1000*10) )(
   .clk() ,
   .rst_in(),
   .rst(),
   .rstn()
   );
*/

module  por_rst #(parameter CNTR_END = 1000*1000*100 )(input clk,rst_in ,output reg rst,output reg  rstn);

reg [31:0] c;
wire of = ( c ==  CNTR_END )  ;

always @ (posedge clk) if (rst_in) c<=0;else if (~of) c<=c+1;
always @ (posedge clk) rst <= ~of ; 
always @ (posedge clk) rstn <= of ;

endmodule 

