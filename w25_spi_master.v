
/*

w25_spi_master w25_spi_master (
.clk       (   ) , 
.rst       (   ) , 
.pin_mosi  (   ) , 
.pin_sclk  (   ) , 
.pin_miso  (   ) , 
.wr        (   ) , 
.busy      (   ) ,
.din       (   ) , 
.dout      (   )   
);

*/
module w25_spi_master (
input clk,rst,
output reg  pin_mosi,pin_sclk,
input pin_miso,
input wr,
output busy,
input [7:0] din,
output reg [7:0] dout 
);


//reg [1:0] d ; always @ (posedge clk) if (rst) d<=0; else d<=d+1;
reg [1:0] d ; always @ (posedge clk) if (rst) d<=0; else d<=(d[1])?0:(d+1);
wire move = d == 0; 

reg [7:0] st ;
reg [7:0] out_r8; always@(posedge clk)  if (wr&st==10) out_r8 <= din ;
reg [7:0] in_r8 ; always@(posedge clk)  if (st==116  ) dout <= in_r8 ;
reg  pin_miso_r ; always@(posedge clk)  if (d==1) pin_miso_r<= pin_miso;

always@(posedge clk)  if (rst) st<=0; else case(st)

0  :  begin  st <= 10 ; pin_mosi<=0;pin_sclk<=0;end  

10 : begin   pin_mosi<=0;pin_sclk<=0; if (  wr  ) st<=20  ;end  
20 :   if ( move ) st<=21  ;
21 :   if ( move ) st<=100 ; 

100: begin   pin_sclk<=0;      pin_mosi<=  out_r8[7] ;  if (move) st<=st+1 ;    end 
101: begin   pin_sclk<=1;      pin_mosi<=  out_r8[7] ;  in_r8[7] <= pin_miso_r;if (move) st<=st+1 ;    end 

102: begin   pin_sclk<=0;      pin_mosi<=  out_r8[6] ;  if (move) st<=st+1 ;    end 
103: begin   pin_sclk<=1;      pin_mosi<=  out_r8[6] ;  in_r8[6] <= pin_miso_r;if (move) st<=st+1 ;    end 

104: begin   pin_sclk<=0;      pin_mosi<=  out_r8[5] ;  if (move) st<=st+1 ;    end 
105: begin   pin_sclk<=1;      pin_mosi<=  out_r8[5] ;  in_r8[5] <= pin_miso_r;if (move) st<=st+1 ;    end 

106: begin   pin_sclk<=0;      pin_mosi<=  out_r8[4] ;  if (move) st<=st+1 ;    end 
107: begin   pin_sclk<=1;      pin_mosi<=  out_r8[4] ;  in_r8[4] <= pin_miso_r;if (move) st<=st+1 ;    end 

108: begin   pin_sclk<=0;      pin_mosi<=  out_r8[3] ;  if (move) st<=st+1 ;    end 
109: begin   pin_sclk<=1;      pin_mosi<=  out_r8[3] ;  in_r8[3] <= pin_miso_r;if (move) st<=st+1 ;    end 

110: begin   pin_sclk<=0;      pin_mosi<=  out_r8[2] ;  if (move) st<=st+1 ;    end 
111: begin   pin_sclk<=1;      pin_mosi<=  out_r8[2] ;  in_r8[2] <= pin_miso_r;if (move) st<=st+1 ;    end 

112: begin   pin_sclk<=0;      pin_mosi<=  out_r8[1] ;  if (move) st<=st+1 ;    end 
113: begin   pin_sclk<=1;      pin_mosi<=  out_r8[1] ;  in_r8[1] <= pin_miso_r;if (move) st<=st+1 ;    end 

114: begin   pin_sclk<=0;      pin_mosi<=  out_r8[0] ;  if (move) st<=st+1 ;    end 
115: begin   pin_sclk<=1;      pin_mosi<=  out_r8[0] ;  in_r8[0] <= pin_miso_r;if (move) st<=st+1 ;    end 

116: begin   pin_sclk<=0; if (move)  st<=st+1;end 
117: begin                if (move)  st<=10; end
default st<=0;
endcase

assign busy = (wr==1) || (st!=10) ;

endmodule 




