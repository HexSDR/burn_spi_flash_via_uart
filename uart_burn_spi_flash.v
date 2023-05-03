
/*

uart_burn_spi_flash  uart_burn_spi_flash (
.clk( ) ,
.rxd( ) ,
.txd( ) ,
.spi_miso( ) ,
.spi_mosi( ) ,
.spi_sck( ) ,
.in32( ) ,
.out32 ( ) 
);

*/

module uart_burn_spi_flash (
input clk,rxd,rst ,
output txd,
input spi_miso,
output spi_mosi,spi_sck,
input [31:0]in32,
output reg [31:0] out32 
);


parameter ClkFrequency = 33333*1000; //33.333MHz
parameter Baud = 115200; 


reg [7:0] st ;
reg [15:0] c,len ;
wire [7:0] rx_u8 ;
wire rx_ready ;

always @ (posedge clk) if (rst) st<=0; else case (st)

0 : st <=5 ;
5 : st<=10 ; 

10 : if (rx_ready) st <= ( rx_u8=='h55)?11:5; 
11 : if (rx_ready) st <= ( rx_u8=='haa)?12:5; 
12 : if (rx_ready) st <= ( rx_u8=='h5a)?13:5; 
13 : if (rx_ready) case (rx_u8)
     20: st<=20 ;   // save 4 bytes and output 
	  30: st<=30 ;   // input 4 bytes 
	  40: st<=40 ;   // write one byte to spi master  
	  50: st<=50 ;   // burst write 
	  60: st<=60 ;   // burset read 
	  default st<= 5;endcase 
	  
	  // output 32 bits 
	  20: if (rx_ready) st<=21; //save q[8*3+7:8*3+0] ;
	  21: if (rx_ready) st<=22; //save q[8*3+7:8*3+0] ;
	  22: if (rx_ready) st<=23; //save q[8*3+7:8*3+0] ;
	  23: if (rx_ready) st<=24; //save q[8*3+7:8*3+0] ;
	  
	  24: st <= 25 ;
	  25: st <= 26 ;
	  26: st <= 27 ; //upload out 
	  27: st <= 5  ;
	  
	  //input 32 bits 
	  30: if (~tx_busy) st <= 31 ; //output d[8*3+7:8*3+0];
	  31: if (~tx_busy) st <= 32 ; //output d[8*3+7:8*3+0];
	  32: if (~tx_busy) st <= 33 ; //output d[8*3+7:8*3+0];
	  33: if (~tx_busy) st <= 34 ; //output d[8*3+7:8*3+0];
	  34: st <= 35 ;
	  35: st <= 36 ;
	  36: st <= 37 ;
	  37: st <= 5  ;
	  
	  //write one bytes to spi 
	  40 : if (rx_ready) st<=41;
	  41 : st<=42; // write to spi master 
	  42 : if (~spi_master_busy) st<=43;
	  43 : if (~tx_busy) st<=44;
	  44: st <= 45 ;
	  45: st <= 5  ;
	  
	  //write len[15:0] bytes to spi without reply .
	  50: if (rx_ready) st <= 51 ; //save q[8*3+7:8*3+0] ;
	  51: if (rx_ready) st <= 52 ; //save q[8*3+7:8*3+0] ;
	  52: st <= 53 ;
	  
	  53:  if (rx_ready)  st<=54;     
      54:  if (~spi_master_busy) st<=55;
	  55:  st<=56;//c++
	  56:  if (c == len )  st<=59 ;else st<=53;
	  58:  st<= 59;
	  59:  st<= 5;
	  
	  60: if ( rx_ready ) st <= 61 ; //save q[8*3+7:8*3+0] ;
	  61: if ( rx_ready ) st <= 62 ; //save q[8*3+7:8*3+0] ;
	  62: if (~spi_master_busy) st <= 63 ; 
	  63: if (~spi_master_busy) st<=65;
	  65: if (~tx_busy) st<=70 ;
	  70: if (c==len) st<=71;else st<=62 ;
	  71: st<=5;
	  
	  default st <= 0 ; 
	  endcase 

reg spi_wr ;

reg [7:0] spi_din ;

always@(posedge clk) if (st==20 && rx_ready==1)out32[8*3+7:8*3+0] <= rx_u8;
always@(posedge clk) if (st==21 && rx_ready==1)out32[8*2+7:8*2+0] <= rx_u8;
always@(posedge clk) if (st==22 && rx_ready==1)out32[8*1+7:8*1+0] <= rx_u8;
always@(posedge clk) if (st==23 && rx_ready==1)out32[8*0+7:8*0+0] <= rx_u8;

always@(posedge clk ) case (st) 50,60 : c <= 0 ;55 : c <= c + 1 ;62 : if (~spi_master_busy) c<=1+c; endcase  
always@(posedge clk ) case (st) 50,60:if (rx_ready)  len[15:8] <= rx_u8;   endcase 
always@(posedge clk ) case (st) 51,61:if (rx_ready)  len[7:0] <= rx_u8;    endcase 

always@(posedge clk ) case (st) 
40,53:   begin spi_wr<=rx_ready;spi_din<=rx_u8 ; end  
62   :   begin spi_wr<= ~spi_master_busy ; spi_din<= 8'hff ; end //burst read from spi 
default  begin spi_wr<=0; end
endcase 

reg [7:0]  tx_d;
reg tx_wr ;

wire [7:0] spi_q;

always@(posedge clk) case (st)
30,31,32,33 : begin tx_wr<=~tx_busy; tx_d<= rx_u8 ;end   
65 : begin tx_wr<=~tx_busy; tx_d<= spi_q ;end  //OK
43  : begin tx_wr<=~tx_busy; tx_d<= spi_q ;end     //OK
default  tx_wr <= 0; endcase 
 
w25_spi_master w25_spi_master (
.clk       (  clk      ) , 
.rst       (  rst      ) , 
.pin_mosi  (  spi_mosi ) , 
.pin_sclk  (  spi_sck  ) , 
.pin_miso  (  spi_miso ) , 
.wr        (  spi_wr   ) , 
.busy      (  spi_master_busy ) ,
.din       (  spi_din  ) , 
.dout      (  spi_q    )   
);

async_transmitter #(  .ClkFrequency(ClkFrequency) ,.Baud(Baud))  async_transmitter (
	.clk(clk),
	.TxD_start(tx_wr),
	.TxD_data(tx_d),
	.TxD(txd),
	.TxD_busy(tx_busy)
);

async_receiver #(  .ClkFrequency(ClkFrequency) ,.Baud(Baud))  async_receiver(
	.clk(clk),
	.RxD(rxd),
	.RxD_data_ready (rx_ready),
	.RxD_data(rx_u8)  // data received, valid only (for one clock cycle) when RxD_data_ready is asserted
 );


endmodule





