




module top(

input clk , 
output pin_mosi,pin_sck,pin_csn,
input pin_miso,

input rxd,
output txd 

);




localparam ClkFrequency = 33333*1000; //33.333MHz
localparam Baud = 115200 *  8 ;   




wire [31:0] out32 ;
assign  pin_csn = out32[0] ;

  por_rst #(.CNTR_END(1000*1000) ) por_rst (
  .clk(clk),
  .rst_in( 1'b0 ) ,
  .rst(rst ),
  .rstn()
  )

uart_burn_spi_flash #(  .ClkFrequency(ClkFrequency) ,.Baud(Baud))    uart_burn_spi_flash (
.clk( clk ) ,
.rst( rst ),
.rxd(  rxd ) ,
.txd(  txd ) ,
.spi_miso(  pin_miso),
.spi_mosi( pin_mosi ) ,
.spi_csn(pin_csn ) ,
.spi_sck( pin_sck ) ,
.in32( 32'h0 ) ,
.out32 ( out32 ) 
);

endmodule 
