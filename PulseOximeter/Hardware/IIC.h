#ifndef __IIC_H
#define __IIC_H

#define IIC_SDA(x)	GPIO_WriteBit(GPIOA, GPIO_Pin_10, (BitAction)(x))
#define IIC_SCL(x)	GPIO_WriteBit(GPIOA, GPIO_Pin_9, (BitAction)(x))
#define IIC_ReadSDA	GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10)

void IIC_Init(void);
void IIC_Start(void);
void IIC_Stop(void);
void IIC_SendByte(uint8_t Byte);
uint8_t IIC_ReceiveByte(void);
void IIC_SendAck(uint8_t Ack);
uint8_t IIC_ReceiveAck(void);

#endif
