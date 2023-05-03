////////////////////////////////////////////////////////
// RS-232 example
// Compiles with Microsoft Visual C++ 5.0/6.0
// (c) fpga4fun.com KNJN LLC - 2003, 2004, 2005, 2006

#include <windows.h>
#include <stdio.h>
#include <conio.h>

////////////////////////////////////////////////////////
HANDLE hCom;

void OpenCom(unsigned int port ,unsigned int baud )
{
	static   char b[10];
	DCB dcb;
	COMMTIMEOUTS ct;
	sprintf(b,"COM%d:",port);
	hCom = CreateFile(b, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hCom==INVALID_HANDLE_VALUE) exit(1);
	if(!SetupComm(hCom, 4096, 4096)) exit(1);
	if(!GetCommState(hCom, &dcb)) exit(1);
	dcb.BaudRate = baud ;
	((unsigned int *)(&dcb))[2] = 0x1001;  // set port properties for TXDI + no flow-control
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = 2;
	if(!SetCommState(hCom, &dcb)) exit(1);	// set the timeouts to 0
	ct.ReadIntervalTimeout = MAXDWORD ;
	ct.ReadTotalTimeoutMultiplier = 0;
	ct.ReadTotalTimeoutConstant = 0;
	ct.WriteTotalTimeoutMultiplier = 0;
	ct.WriteTotalTimeoutConstant = 0;
	if(!SetCommTimeouts(hCom, &ct)) exit(1);
	printf("ok to open %s with %d \n",b,baud  );
}

void CloseCom()
{
	CloseHandle(hCom);
}

unsigned int  WriteCom(unsigned char* buf, int len)
{
	unsigned int  nSend;
	if(!WriteFile(hCom, buf, len, &nSend, NULL)) exit(1);
	return nSend;
} 

void WriteComChar(unsigned char b)
{
	WriteCom(&b, 1);
}

unsigned char  ReadCom(unsigned char *buf, int len)
{
	unsigned char  nRec;
	if(!ReadFile(hCom, buf, len, &nRec, NULL)) exit(1);
	return (unsigned char )nRec;
}

unsigned char ReadComChar()
{
	unsigned int  nRec;	char c;  
	do {
			if(!ReadFile(hCom, &c, 1, &nRec, NULL)) exit(1);
		 
	}
	while (nRec==0    ) ; 
	return   c  ;
}

////////////////////////////////////////////////////////


/*

      20: st<=20 ;   // save 4 bytes and output 
	  30: st<=30 ;   // input 4 bytes 
	  40: st<=40 ;   // write one byte to spi master  
	  50: st<=50 ;   // burst write 
	  60: st<=60 ;   // burset read 
	  
*/

int do_cmd_wr32(unsigned int u32 ){       //  20 
	static unsigned char tmp[1024*4] ;
	int t,i=0 ;
	tmp[i++] = 0x55;
	tmp[i++] = 0xaa ;
	tmp[i++] = 0x5a  ;
	tmp[i++] = 20 ;//burst write 
	tmp[i++] = ( unsigned char ) ( ( u32 >>24 )    &  0xff  ) ;  //burst write 
	tmp[i++] = ( unsigned char ) ( ( u32 >>16 )    &  0xff  ) ;  //burst write 	
	tmp[i++] = ( unsigned char ) ( ( u32 >>8  )    &  0xff  ) ;  //burst write 	
	tmp[i++] = ( unsigned char ) ( ( u32      )    &  0xff  ) ;  //burst write 	
	WriteCom( tmp,i );	
}

unsigned int do_cmd_rd32 (){ //  30
	static  unsigned char  tmp[1024*4] ;
	unsigned  int t,i=0 ;
	tmp[i++] = 0x55;
	tmp[i++] = 0xaa ;
	tmp[i++] = 0x5a  ;
	tmp[i++] = 30  ; 
	WriteCom( tmp,i );
	tmp[ 0 ] = ReadComChar();
	tmp[ 1 ] = ReadComChar();
	tmp[ 2 ] = ReadComChar();
	tmp[ 3 ] = ReadComChar(); 
	t  = tmp[0] ; t <<= 8;
	t |= tmp[1] ; t <<= 8;
	t |= tmp[2] ; t <<= 8;
	t |= tmp[3] ;  
	return t; 	
}

unsigned char do_cmd_wr_rd_u8 (unsigned char u8 ){//40 
 static unsigned  char tmp[1024*4] ;
	int t,i=0 ;
	tmp[i++] = 0x55;
	tmp[i++] = 0xaa ;
	tmp[i++] = 0x5a  ;
	tmp[i++] = 40   ;//burst write 
	tmp[i++] = u8   ;//burst write 
	WriteCom( tmp,i );
			tmp[ 0 ] = ReadComChar();   
	return tmp[0] ;
}

int do_cmd_burst_wr(unsigned char *b,int len){ // 50 
	static unsigned  char  tmp[1024*4] ;
	int t,i=0 ;
	tmp[i++] = 0x55  ;
	tmp[i++] = 0xaa  ;
	tmp[i++] = 0x5a  ;
	tmp[i++] = 50    ; //burst write 
	tmp[i++] = (len >>8) &0xff ;
	tmp[i++] = len &0xff ;
	for(t=0;t<len;++t) tmp[i++] = b[t];
	WriteCom( tmp,i );	
}

int do_cmd_burst_rd(unsigned char *b,int len){ // 60 
	static unsigned  char  tmp[1024*64] ;
	int t,i=0 ;
	tmp[i++] = 0x55  ;
	tmp[i++] = 0xaa  ;
	tmp[i++] = 0x5a  ;
	tmp[i++] = 60    ; //burst read
	tmp[i++] = (len >>8) &0xff ;
	tmp[i++] = len &0xff ;
	WriteCom( tmp,i );
	for(t=0;t<len;++t) b[t] = ReadComChar();
	return len ;
}


static unsigned int idx  = 0 ;
static unsigned char wr_buff[1024*4] ;

void spi_wr_u8 (unsigned char u8){
	wr_buff[idx++]=u8;
}

void spi_wr_flush (){
	if ( idx == 0 ) return ;
	do_cmd_burst_wr(wr_buff,idx) ;
	idx = 0 ;
}

unsigned char  spi_wr_rd_u8 (unsigned char u8 ){
	spi_wr_flush() ; 
	return do_cmd_wr_rd_u8(u8 );
}
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int  out32  = 0xffff ;

void  FLASH_CS_CLR(){
	spi_wr_flush();
	out32 &=  ~ (1<<0) ;  
	do_cmd_wr32 ( out32 ) ;
} 

void  FLASH_CS_SET(){
	spi_wr_flush();
	out32 |=   (1<<0) ;  
	do_cmd_wr32 ( out32 ) ;
} 

unsigned char SPI_ReadWriteByte(unsigned char u8 ){
		spi_wr_flush();
	return spi_wr_rd_u8(u8);	
}

void SPI_WriteByte(unsigned char u8 ){
 spi_wr_u8(u8);	
	//		spi_wr_flush();
//		return spi_wr_rd_u8(u8);
}
 
   
#define W25X_WriteEnable        0x06
#define W25X_WriteDisable       0x04
#define W25X_ReadStatusReg      0x05
#define W25X_WriteStatusReg     0x01
#define W25X_ReadData           0x03
#define W25X_FastReadData       0x0B
#define W25X_FastReadDual       0x3B
#define W25X_PageProgram        0x02
#define W25X_BlockErase         0xD8
#define W25X_SectorErase        0x20
#define W25X_ChipErase          0xC7
#define W25X_PowerDown          0xB9
#define W25X_ReleasePowerDown   0xAB
#define W25X_DeviceID           0xAB
#define W25X_ManufactDeviceID   0x90
#define W25X_JedecDeviceID      0x9F


unsigned char  SPI_Flash_ReadSR(){
    unsigned char  byte=0;
    FLASH_CS_CLR();                           
    SPI_ReadWriteByte(W25X_ReadStatusReg);   
    byte = SPI_ReadWriteByte(0Xff);             
    FLASH_CS_SET();                           
	return byte;
}

void SPI_Flash_Wait_Busy(void){
    while((SPI_Flash_ReadSR()&0x01)==0x01);
}

void SPI_FLASH_Write_SR(unsigned char  sr){
    FLASH_CS_CLR();                          
    SPI_ReadWriteByte(W25X_WriteStatusReg);  
    SPI_ReadWriteByte(sr);             
    FLASH_CS_SET();                       
}

 
void SPI_FLASH_Write_Enable(void){
    FLASH_CS_CLR();                            
    SPI_ReadWriteByte(W25X_WriteEnable);     
    FLASH_CS_SET();                           
}

 
void SPI_FLASH_Write_Disable(void){
    FLASH_CS_CLR();                        
    SPI_ReadWriteByte(W25X_WriteDisable);    
    FLASH_CS_SET();                            
}


//0XEF13 W25Q80
//0XEF14 W25Q16
//0XEF15 W25Q32
//0XEF16 W25Q64
//0XEF17 W25Q128

unsigned short  SPI_Flash_ReadID(void){
    unsigned short  Temp = 0;
    FLASH_CS_CLR();
    SPI_ReadWriteByte( 0x90 );
    SPI_ReadWriteByte( 0x00 );
    SPI_ReadWriteByte( 0x00 );
    SPI_ReadWriteByte( 0x00 ); 
    Temp|=SPI_ReadWriteByte( 0xFF ) << 8 ;
    Temp|=SPI_ReadWriteByte( 0xFF );
    FLASH_CS_SET();
    printf(" SPI_Flash_ReadID return value is %04x \n", Temp ) ;   
    return Temp;
}
 

void SPI_Flash_Read(unsigned char * pBuffer,unsigned int  ReadAddr,unsigned short  NumByteToRead){
    unsigned short  i; 
    FLASH_CS_CLR();                             
    SPI_ReadWriteByte(W25X_ReadData);          
    SPI_ReadWriteByte((unsigned char )((ReadAddr)>>16));  
    SPI_ReadWriteByte((unsigned char )((ReadAddr)>>8));
    SPI_ReadWriteByte((unsigned char )ReadAddr);
    
    do_cmd_burst_rd(pBuffer , NumByteToRead)   ;
    
    
	 /*
    for(i=0; i<NumByteToRead; i++){
        pBuffer[i]=SPI_ReadWriteByte(0XFF);    
    }
    */
    
	 
    FLASH_CS_SET();
}

void SPI_Flash_Write_Page(unsigned char * pBuffer,unsigned int  WriteAddr,unsigned short  NumByteToWrite){//NumByteToWrite=256 
    unsigned short  i;
    SPI_FLASH_Write_Enable();                  
    FLASH_CS_CLR();                              
    SPI_WriteByte(W25X_PageProgram);       
    SPI_WriteByte((unsigned char )((WriteAddr)>>16));  
    SPI_WriteByte((unsigned char )((WriteAddr)>>8));
    SPI_WriteByte((unsigned char )WriteAddr);
    for(i=0; i<NumByteToWrite; i++){ 
        SPI_WriteByte(pBuffer[i]);
    }
    FLASH_CS_SET();                           
    SPI_Flash_Wait_Busy();                    
}

 
void SPI_Flash_Erase_Chip(void)
{
    SPI_FLASH_Write_Enable();                 
    SPI_Flash_Wait_Busy();
    FLASH_CS_CLR();                            
    SPI_ReadWriteByte(W25X_ChipErase);        
    FLASH_CS_SET();                             
    SPI_Flash_Wait_Busy();                     
} 

void SPI_Flash_PowerDown(void)
{
    int i;
    FLASH_CS_CLR();                            
    SPI_ReadWriteByte(W25X_PowerDown);         
    FLASH_CS_SET();                           
    i = 255;    while(i--);
}

void SPI_Flash_WAKEUP(void)
{
    int i ;
    FLASH_CS_CLR();                             
    SPI_ReadWriteByte(W25X_ReleasePowerDown);    
    FLASH_CS_SET();                             
    i = 255;    while(i--);
}
 
#define BUFF_SIZE 1024*1024*16
static unsigned char file_buff[ BUFF_SIZE ] ;
static unsigned char  rcv_buff[ BUFF_SIZE ] ;
static int file2buff(unsigned char *file_buff,  char *fn){
	int r,i ;	FILE *fp ; 
	fp = fopen( fn , "rb" );	
	if (fp==NULL) printf("can not open file (%s)\n",fn);
	if (fp==NULL) {return 0;exit(1); } 
	r = fread( file_buff , 1, BUFF_SIZE ,fp ); 
	fclose (fp); 
	printf("%s get file size is %d\r",fn, r ); 
	return r ;	
}
 
unsigned int  burn_flash (char * fn ){
unsigned int  i=0, r, remain ;
	printf("start to erase chip\n");
	SPI_Flash_Erase_Chip();
	printf("erase chip done \n");
	for(i=0;i<BUFF_SIZE;++i) file_buff[i] = 0xff ;i=0; 
	remain =r= file2buff(file_buff,fn);	
	printf("\n" ); 	
	float f ,f1,f2;	
	while(1){
		if (remain < 256)break;
		SPI_Flash_Write_Page(file_buff+i,i,256);
		f1 = i*100.0;
		f2 = r*1.0;		
		f=f1/f2;
		printf("Burn addr is %d of %d     [%2.2f%] \r",  i,r,(float)f ); fflush(stdout);
		remain -= 256 ; i += 256 ;  
	}
    if (remain) printf("burn last %d bytes\n",remain);
	if (remain) SPI_Flash_Write_Page(file_buff+i,i,256); 
	return r ; 
}

#define BLOCK_SIZE 8192
unsigned int    read_flash(unsigned char *rcv_buff , unsigned int  len ){
unsigned int  r, i,s  = 0;i=0;
printf("read flash start len = %d  \n",len);
while(i<len){
	s = len - i ;
	s = (s>BLOCK_SIZE)?BLOCK_SIZE:s;
	printf("read address is %d\r",i);
	SPI_Flash_Read(rcv_buff+i,i,s); 
	i+=s;
} 
printf("\nread flash done \n");
}

int my_memcmp(unsigned char *a,unsigned  char *b,int s){
int i = 0 ;for(i=0;i<s;++i){if (a[i]!=b[i]) return i+1;}
return 0;
}

int  main()
{ int c=0;
	int  r , len,t;
	OpenCom(  4 , 115200  *8 );
//	while(1)]
//	{
	FLASH_CS_SET();
//	sleep(1);
//	FLASH_CS_CLR();     
//	sleep(1);
//	}

 
t=0;
///while(1)
{
	
	
	
//	t++;
// 	SPI_ReadWriteByte(0) ; 	SPI_ReadWriteByte(0) ; 	SPI_ReadWriteByte(0) ; 

//	r = 	SPI_ReadWriteByte(t) ; 
//	printf("r=%d\n",r); 
}

 
 

start :
	r = SPI_Flash_ReadID();
	
	printf("Flash id is %08x\n" ,r );
	
	
//	getchar() ;	getchar() ;	getchar() ;	getchar() ;	getchar() ;	getchar() ;	getchar() ;


		//	len = burn_flash("com.c");
						len = burn_flash("b200.ucf");
			
						 printf("start read \n");
	read_flash( rcv_buff,len );
	
				 printf("Done  read \n");
 
	r  = my_memcmp( rcv_buff , file_buff , len );
	if (r==0){		
	
		printf("verify OK %d \n",c++);goto start ;
	} else {
		printf("verify ERROR @ 0x%X c=%d\n ",r,c);  getchar();getchar();getchar();getchar();getchar(); 
	}
	
	
	
	CloseCom();
	
}

