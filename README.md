# burn_spi_flash_via_uart
burn SPI flash via UART

这里提供了一个通过串口烧写SPI FLASH的方案，包含HDL(全部使用状态机)和WINDOWS系统上运行的C语言。


其中串口通讯部分的C和HDL是来自 [www.fpga4fun.com ](https://www.fpga4fun.com/SerialInterface.html)
其他代码本人纯手工打造。
尤其是有限状态机FSM的使用，基本体现了本人使用使用FSM串行处理的风格。


使用时候注意修改一下时钟
参数ClkFrequency 定义为你的板子上的时钟频率。

参数Baud 我们使用的是115200*8 (即921600)，如果要修改，请对照com.c同时修改。


com.c我使用dev_cpp编译运行。
HDL代码我使用ISE编译在SPARTAN 6板子上运行OK。纯粹的RTL代码，不会挑FPGA型号和开发环境的。
使用W25Q64进行读写测试通过。


这些代码除了直接使用于具体项目，可以作为初学者研究学习使用状态机编程的例子非常不错。





