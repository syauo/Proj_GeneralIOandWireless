#include "./NRF24L01/24l01.h"

    
const uint8_t TX_ADDRESS[TX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //发送地址
const uint8_t RX_ADDRESS[RX_ADR_WIDTH]={0x34,0x43,0x10,0x10,0x01}; //发送地址
static __IO uint32_t  SPITimeout = SPIT_LONG_TIMEOUT;    
static uint16_t SPI_TIMEOUT_UserCallback(uint8_t errorCode);

//初始化24L01的IO口
void NRF24L01_Config(void)
{  
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure; 
    
    /* 使能SPI时钟 */
    NRF_SPI_APBxClock_FUN( NRF_SPI_CLK, ENABLE );
    
    /* 使能SPI引脚相关的时钟 */
    NRF_SPI_CSN_APBxClock_FUN( NRF_SPI_CSN_CLK, ENABLE );
    NRF_SPI_SCK_APBxClock_FUN( NRF_SPI_SCK_CLK | NRF_SPI_MISO_CLK | NRF_SPI_MOSI_CLK, ENABLE);
    NRF_CE_APBxClock_FUN( NRF_CE_CLK, ENABLE);
    NRF_IRQ_APBxClock_FUN( NRF_IRQ_CLK, ENABLE);

    /* 配置SPI的 CSN引脚，普通IO */
    GPIO_InitStructure.GPIO_Pin = NRF_SPI_CSN_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // 推挽输出
    GPIO_Init(NRF_SPI_CSN_PORT, &GPIO_InitStructure);

    /* 配置NRF的CE引脚 */
    GPIO_InitStructure.GPIO_Pin = NRF_CE_PIN;
    GPIO_Init(NRF_CE_PORT, &GPIO_InitStructure);

    /* 配置SPI的 SCK引脚*/
    GPIO_InitStructure.GPIO_Pin = NRF_SPI_SCK_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;     // 复用推挽输出
    GPIO_Init(NRF_SPI_SCK_PORT, &GPIO_InitStructure);

    /* 配置SPI的 MISO引脚*/
    GPIO_InitStructure.GPIO_Pin = NRF_SPI_MISO_PIN;
    GPIO_Init(NRF_SPI_MISO_PORT, &GPIO_InitStructure);

    /* 配置SPI的 MOSI引脚*/
    GPIO_InitStructure.GPIO_Pin = NRF_SPI_MOSI_PIN;     
    GPIO_Init(NRF_SPI_MOSI_PORT, &GPIO_InitStructure);

    /* 配置NRF的IRQ引脚 */
    GPIO_InitStructure.GPIO_Pin = NRF_IRQ_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU  ;   //上拉输入
    GPIO_Init(NRF_IRQ_PORT, &GPIO_InitStructure);
    /* 使能NRF CE=0*/
    NRF_CE_LOW();
    /* 停止信号 CSN引脚高电平*/
    SPI_NRF_CSN_HIGH();

    /* SPI 模式配置 */
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;        //设置SPI工作模式:设置为主SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;        //设置SPI的数据大小:SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;        //选择了串行时钟的稳态:时钟悬空低电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;    //数据捕获于第一个时钟沿
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;        //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;        //定义波特率预分频的值
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;    //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7;    //CRC值计算的多项式
    SPI_Init(NRF_SPIx, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
    /* 使能 SPI  */
    SPI_Cmd(NRF_SPIx , ENABLE);
}

//检测24L01是否存在
//返回值:0-成功;1-失败    
uint8_t NRF24L01_Check(void)
{
    uint8_t buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
    uint8_t i;
    ////spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）        
    //xw- SPI1_SetSpeed(SPI_BaudRatePrescaler_8); 
    //写入5个字节的地址.    
    NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,buf,5);
    //读出写入的地址  
    NRF24L01_Read_Buf(TX_ADDR,buf,5); 
    for(i=0;i<5;i++){
      if(buf[i]!=0XA5){
        break;                                    
      }
    }
    if(i!=5){
      return 1;   //检测24L01错误    
    }
    return 0;     //检测到24L01
}          

//SPI写寄存器
//reg:指定寄存器地址
//value:写入的值
uint8_t NRF24L01_Write_Reg(uint8_t reg,uint8_t value)
{
    uint8_t status;                    
    SPI_NRF_CSN_LOW();                    //使能SPI传输
    status =NRF24L01_SPI_SendByte(reg);   //发送寄存器号 
    NRF24L01_SPI_SendByte(value);         //写入寄存器的值
    SPI_NRF_CSN_HIGH();                   //禁止SPI传输       
    return(status);                       //返回状态值
}

//读取SPI寄存器值
//reg:要读的寄存器
uint8_t NRF24L01_Read_Reg(uint8_t reg)
{
    uint8_t reg_val;        
    SPI_NRF_CSN_LOW();                    //使能SPI传输        
    NRF24L01_SPI_SendByte(reg);           //发送寄存器号
    reg_val=NRF24L01_SPI_ReadByte();      //读取寄存器内容
    SPI_NRF_CSN_HIGH();                   //禁止SPI传输            
    return(reg_val);                      //返回状态值
}    

//在指定位置读出指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值 
uint8_t NRF24L01_Read_Buf(uint8_t reg,uint8_t *pBuf,uint8_t len)
{
    uint8_t status,u8_ctr;           
    SPI_NRF_CSN_LOW();                    //使能SPI传输
    status=NRF24L01_SPI_SendByte(reg);    //发送寄存器值(位置),并读取状态值          
    for(u8_ctr=0;u8_ctr<len;u8_ctr++){
      pBuf[u8_ctr]=NRF24L01_SPI_ReadByte(); //读出数据
    }
    SPI_NRF_CSN_HIGH();                   //关闭SPI传输
    return status;                        //返回读到的状态值
}

//在指定位置写指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值
uint8_t NRF24L01_Write_Buf(uint8_t reg, uint8_t *pBuf, uint8_t len)
{
    uint8_t status,u8_ctr;        
    SPI_NRF_CSN_LOW();                    //使能SPI传输
    status = NRF24L01_SPI_SendByte(reg);  //发送寄存器值(位置),并读取状态值
    for(u8_ctr=0; u8_ctr<len; u8_ctr++){
      NRF24L01_SPI_SendByte(*pBuf++);     //写入数据     
    }
    SPI_NRF_CSN_HIGH();                   //关闭SPI传输
    return status;                        //返回读到的状态值
}    

//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:发送完成状况
uint8_t NRF24L01_TxPacket(uint8_t *txbuf)
{
    uint8_t sta;
    ////spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   
    //-xw SPI1_SetSpeed(SPI_BaudRatePrescaler_8);
    NRF_CE_LOW();
    //写数据到TX BUF  32个字节
    NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);
    NRF_CE_HIGH();                        //启动发送       
    while(NRF_IRQ!=0);                    //等待发送完成
    sta=NRF24L01_Read_Reg(STATUS);        //读取状态寄存器的值       
    //清除TX_DS或MAX_RT中断标志
    NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); 
    //达到最大重发次数
    if(sta&MAX_TX){
        NRF24L01_Write_Reg(FLUSH_TX,0xff);//清除TX FIFO寄存器 
        return MAX_TX; 
    }
    //发送完成
    if(sta&TX_OK){
        return TX_OK;
    }
    //其他原因发送失败
    return 0xff;
}

//启动NRF24L01接收一次数据
//rxbuf:接收数据存放首地址
//返回值:0，接收完成；其他，错误代码
uint8_t NRF24L01_RxPacket(uint8_t *rxbuf)
{
    uint8_t sta;                                           
    ////spi速度为9Mhz（24L01的最大SPI时钟为10Mhz）   
    //-xw SPI1_SetSpeed(SPI_BaudRatePrescaler_8); 
    //读取状态寄存器的值         
    sta=NRF24L01_Read_Reg(STATUS);  
    //清除TX_DS或MAX_RT中断标志
    NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); 
    //接收到数据
    if(sta&RX_OK){
        //读取数据
        NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);
        //清除RX FIFO寄存器 
        NRF24L01_Write_Reg(FLUSH_RX,0xff);
        return 0; 
    }       
    //没收到任何数据
    return 1;
}   

//初始化NRF24L01到RX模式
//设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR
//当CE变高后,即进入RX模式,并可以接收数据了           
void NRF24L01_RX_Mode(void)
{
    NRF_CE_LOW();      
    //写RX节点地址
    NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,
                       (u8*)RX_ADDRESS,RX_ADR_WIDTH);
  
    //使能通道0的自动应答    
    NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);        
    //使能通道0的接收地址       
    NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01);
    //设置RF通信频率          
    NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,40);            
    //选择通道0的有效数据宽度         
    NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);
    //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
    NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);    
    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 
    NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0f);        
    //CE为高,进入接收模式 
    NRF_CE_HIGH(); 
}      

//初始化NRF24L01到TX模式
//设置TX地址,写TX数据宽度,设置RX自动应答的地址,填充TX发送数据,选择RF频道,波特率和LNA HCURR
//PWR_UP,CRC使能
//当CE变高后,即进入RX模式,并可以接收数据了           
//CE为高大于10us,则启动发送.     
void NRF24L01_TX_Mode(void)
{                                                         
    NRF_CE_LOW();        
    //写TX节点地址 
    NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH);
    //设置TX节点地址,主要为了使能ACK      
    NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH); 

    //使能通道0的自动应答    
    NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);     
    //使能通道0的接收地址  
    NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01); 
    //设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
    NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_RETR,0x1a);
    //设置RF通道为40
    NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,40);       
    //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
    NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);  
    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
    NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,0x0e);    
    //CE为高,10us后启动发送
    NRF_CE_HIGH();
}

/*以下内容引用自Fire Tutorial--------------------------------------------------------*/

 /**
  * @brief  使用SPI读取一个字节的数据
  * @param  无
  * @retval 返回接收到的数据
  */
uint8_t NRF24L01_SPI_ReadByte(void)
{
  return (NRF24L01_SPI_SendByte(Dummy_Byte));
}

 /**
  * @brief  使用SPI发送一个字节的数据
  * @param  byte：要发送的数据
  * @retval 返回接收到的数据
  */
uint8_t NRF24L01_SPI_SendByte(uint8_t byte)
{
   SPITimeout = SPIT_FLAG_TIMEOUT;
  /* 等待发送缓冲区为空，TXE事件 */
  while (SPI_I2S_GetFlagStatus(NRF_SPIx , SPI_I2S_FLAG_TXE) == RESET)
  {
    if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(0);
   }

  /* 写入数据寄存器，把要写入的数据写入发送缓冲区 */
  SPI_I2S_SendData(NRF_SPIx, byte);

  SPITimeout = SPIT_FLAG_TIMEOUT;
  /* 等待接收缓冲区非空，RXNE事件 */
  while (SPI_I2S_GetFlagStatus(NRF_SPIx, SPI_I2S_FLAG_RXNE) == RESET)
  {
    if((SPITimeout--) == 0) return SPI_TIMEOUT_UserCallback(1);
   }

  /* 读取数据寄存器，获取接收缓冲区数据 */
  return SPI_I2S_ReceiveData(NRF_SPIx );
}

static  uint16_t SPI_TIMEOUT_UserCallback(uint8_t errorCode)
{
  /* 等待超时后的处理,输出错误信息 */
  FLASH_ERROR("SPI 等待超时!errorCode = %d",errorCode);
  return 0;
}





