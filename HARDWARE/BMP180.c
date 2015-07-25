#include "BMP180.h"
#include "stm32f4xx.h"
#include "delay.h"


/*
	BMP180 ��BOSCH��˾�����ĵ�һ��߾��ȡ������ܺĵ�ѹ�������������Ծ�����Ϳ��Դﵽ0.03hPa��
	BMP180 ͨ��I2C����ֱ�������΢������������
	ѹ����Χ��300 ... 1100hPa������9000��...-500�ף�
			30kPa ... 110kPa

	ѹǿ��λ��
		inHg Ӣ�繯��
		mmHg ���׹���
		mbar ����(=����)
		hPa ����
		kPa ǧ��, 1kPa = 1000Pa
		MPa ����, 1MPa = 1000kPa = 1000000Pa

	1����=1����=3/4����ˮ����
	�ں�ƽ���ƽ����ѹ ԼΪ1013.25����˹����760����ˮ�����������ֵҲ����Ϊ��׼����ѹ

	�����⻻�㣺
		1��׼����ѹ=101 325��˹��
		1��˹��=9.8692326671601*10-6��׼����ѹ

	Doc�ļ������С�ȫ��������Ҫ���к��θ߶ȼ�����ѹ�ο�����.pdf��

	����   ���θ߶�(m)  ��ѹǿ��(kPa)
	����     31.2         99.86
	���     3.3          100.48
	ʯ��ׯ   80.5         99.56
	̫ԭ     777.9        91.92
	���ͺ��� 1063         88.94
	����     41.6         100.07
	����     92.8         99.47
	����     236.8        97.79
	������   171.7        98.51
	�Ϻ�     4.5          100.53
	�Ͼ�     8.9          100.4
	����     41.7         100.05
	�Ϸ�     29.8         100.09
	����     84           99.64
	����     63.2         99.91
	�ϲ�     46.7         99.91
	����     51.6         99.85
	�人     23.3         100.17
	֣��     110.4        99.17
	��ɳ     44.9         99.94
	����     6.6          100.45
	����     72.2         99.6
	����     259.1        97.32
	����     1071.2       88.79
	����     1891.4       80.8
	����     3658.        65.23
	����     396.9        95.92
	����     1517.2       84.31
	�ɶ�     505.9        94.77
	����     2261.2       77.35
	����     1111.5       88.35
	��³ľ�� 917.9        90.67

	���     32           100.56
	̨��     9            100.53

	��ͷ     1.2          100.55   ����ͺ��Ρ��㶫ʡ
	����     4507         58.9     ����ߺ��Ρ�����������

	���߶�Ϊ���� 8848��    �൱��0.3������ѹ ,��30.39KPa  �������ݴ������顿

	��������ѹ���¶ȼ��㺣�θ߶ȣ�ֻ�Ǵ��¹�ϵ�������ο���
	H = (RT/gM)*ln(p0/p)
		RΪ����8.51
		TΪ����ѧ�¶ȣ������£������϶�Ҫת��������ѧ�¶ȣ�
		gΪ�������ٶ�10
		MΪ����ķ�����29
		P0Ϊ��׼����ѹ
		PΪҪ��ĸ߶ȵ���ѹ
		�����ʽ���Ƶ����̽ϸ��ӾͲ��Ƶ��ˣ��Լ�Ӧ�û�ת���ġ�


	�����ܽ�����Ƶļ��㹫ʽ��
		P=100KPa   H*10kPa/km   H��0,3km֮��
		P=70kPa    H*8kPa/km    H��3km,5km֮��
		P=54kPa    H*6.5kPa/km   H��5km,7km֮��
		P=41kPa    H*5kPa/km   H��7km,10km֮��
		P=25kPa    H*3.5kPa/km   H��10km,12km֮��

*/

static void BMP180_WriteReg(uint8_t _ucRegAddr, uint8_t _ucRegValue);
static uint16_t BMP180_Read2Bytes(uint8_t _ucRegAddr);
static uint32_t BMP180_Read3Bytes(uint8_t _ucRegAddr);
static void BMP180_WaitConvert(void);

BMP180_T g_tBMP180;

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitBMP180
*	����˵��: ��ʼ��BMP180
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void BMP180_Init(void)
{
	/* ����оƬ�ڲ���У׼������ÿ��оƬ��ͬ������BOSCH����ǰУ׼�õ����ݣ� */
	g_tBMP180.AC1 = (int16_t)BMP180_Read2Bytes(0xAA);
	g_tBMP180.AC2 = (int16_t)BMP180_Read2Bytes(0xAC);
	g_tBMP180.AC3 = (int16_t)BMP180_Read2Bytes(0xAE);
	g_tBMP180.AC4 = (uint16_t)BMP180_Read2Bytes(0xB0);
	g_tBMP180.AC5 = (uint16_t)BMP180_Read2Bytes(0xB2);
	g_tBMP180.AC6 = (uint16_t)BMP180_Read2Bytes(0xB4);
	g_tBMP180.B1 =  (int16_t)BMP180_Read2Bytes(0xB6);
	g_tBMP180.B2 =  (int16_t)BMP180_Read2Bytes(0xB8);
	g_tBMP180.MB =  (int16_t)BMP180_Read2Bytes(0xBA);
	g_tBMP180.MC =  (int16_t)BMP180_Read2Bytes(0xBC);
	g_tBMP180.MD =  (int16_t)BMP180_Read2Bytes(0xBE);

	g_tBMP180.OSS = 0;	/* ������������0-3 */
	BMP180_InitI2C();
}

/*
*********************************************************************************************************
*	�� �� ��: BMP180_WriteReg
*	����˵��: д�Ĵ���
*	��    ��: _ucOpecode : �Ĵ�����ַ
*			  _ucRegData : �Ĵ�������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void BMP180_WriteReg(uint8_t _ucRegAddr, uint8_t _ucRegValue)
{
    BMP180_i2c_Start();							/* ���߿�ʼ�ź� */

    BMP180_i2c_SendByte(BMP180_SLAVE_ADDRESS);		/* �����豸��ַ+д�ź� */
	BMP180_i2c_WaitAck();

    BMP180_i2c_SendByte(_ucRegAddr);				/* ���ͼĴ�����ַ */
	BMP180_i2c_WaitAck();

    BMP180_i2c_SendByte(_ucRegValue);				/* ���ͼĴ�����ֵ */
	BMP180_i2c_WaitAck();

    BMP180_i2c_Stop();                   			/* ����ֹͣ�ź� */
}

/*
*********************************************************************************************************
*	�� �� ��: BMP180_Read2Bytes
*	����˵��: ��ȡBMP180�Ĵ�����ֵ������2�ֽڡ������¶ȼĴ���
*	��    ��: _ucRegAddr �Ĵ�����ַ
*	�� �� ֵ: �Ĵ���ֵ
*********************************************************************************************************
*/
static uint16_t BMP180_Read2Bytes(uint8_t _ucRegAddr)
{
	uint8_t ucData1;
	uint8_t ucData2;
	uint16_t usRegValue;

	BMP180_i2c_Start();                  			/* ���߿�ʼ�ź� */
	BMP180_i2c_SendByte(BMP180_SLAVE_ADDRESS);		/* �����豸��ַ+д�ź� */
	BMP180_i2c_WaitAck();
	BMP180_i2c_SendByte(_ucRegAddr);				/* ���͵�ַ */
	BMP180_i2c_WaitAck();

	BMP180_i2c_Start();                  			/* ���߿�ʼ�ź� */
	BMP180_i2c_SendByte(BMP180_SLAVE_ADDRESS + 1);/* �����豸��ַ+���ź� */
	BMP180_i2c_WaitAck();

	ucData1 = BMP180_i2c_ReadByte();       		/* �������ֽ����� */
	BMP180_i2c_Ack();

	ucData2 = BMP180_i2c_ReadByte();       		/* �������ֽ����� */
	BMP180_i2c_NAck();
	BMP180_i2c_Stop();                  			/* ����ֹͣ�ź� */

	usRegValue = (ucData1 << 8) + ucData2;

	return usRegValue;
}

/*
*********************************************************************************************************
*	�� �� ��: BMP180_Read3Bytes
*	����˵��: ��ȡBMP180�Ĵ�����ֵ������3�ֽ�  ���ڶ�ѹ���Ĵ���
*	��    ��: _ucRegAddr �Ĵ�����ַ
*	�� �� ֵ: �Ĵ���ֵ
*********************************************************************************************************
*/
static uint32_t BMP180_Read3Bytes(uint8_t _ucRegAddr)
{
	uint8_t ucData1;
	uint8_t ucData2;
	uint8_t ucData3;
	uint32_t uiRegValue;

	BMP180_i2c_Start();                  			/* ���߿�ʼ�ź� */
	BMP180_i2c_SendByte(BMP180_SLAVE_ADDRESS);		/* �����豸��ַ+д�ź� */
	BMP180_i2c_WaitAck();
	BMP180_i2c_SendByte(_ucRegAddr);				/* ���͵�ַ */
	BMP180_i2c_WaitAck();

	BMP180_i2c_Start();                  			/* ���߿�ʼ�ź� */
	BMP180_i2c_SendByte(BMP180_SLAVE_ADDRESS + 1);/* �����豸��ַ+���ź� */
	BMP180_i2c_WaitAck();

	ucData1 = BMP180_i2c_ReadByte();       		/* �������ֽ����� */
	BMP180_i2c_Ack();

	ucData2 = BMP180_i2c_ReadByte();       		/* �����м��ֽ����� */
	BMP180_i2c_Ack();

	ucData3 = BMP180_i2c_ReadByte();       		/* ������ͽ����� */
	BMP180_i2c_NAck();
	BMP180_i2c_Stop();                  			/* ����ֹͣ�ź� */

	uiRegValue = (ucData1 << 16) + (ucData2 << 8) + ucData3;

	return uiRegValue;
}

/*
*********************************************************************************************************
*	�� �� ��: BMP180_WaitConvert
*	����˵��: �ȴ��ڲ�ת������
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void BMP180_WaitConvert(void)
{
	if (g_tBMP180.OSS == 0)
	{
		delay_ms(6);		/* 4.5ms  7.5ms  13.5ms   25.5ms */
	}
	else if (g_tBMP180.OSS == 1)
	{
		delay_ms(9);		/* 4.5ms  7.5ms  13.5ms   25.5ms */
	}
	else if (g_tBMP180.OSS == 2)
	{
		delay_ms(15);	/* 4.5ms  7.5ms  13.5ms   25.5ms */
	}
	else if (g_tBMP180.OSS == 3)
	{
		delay_ms(27);	/* 4.5ms  7.5ms  13.5ms   25.5ms */
	}
}

/*
*********************************************************************************************************
*	�� �� ��: BMP180_ReadTempPress
*	����˵��: ��ȡBMP180�������¶�ֵ��ѹ��ֵ����������ȫ�ֱ��� g_tBMP180.Temp��g_tBMP180.Press
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void BMP180_ReadTempPress(void)
{
	long UT, X1, X2, B5, T;
	long UP, X3, B3, B6, B7, p;
	unsigned long B4;

	/* ���̼� pdf page 12 */

	/* ���¶�ԭʼֵ */
	BMP180_WriteReg(0xF4, 0x2E);
	BMP180_WaitConvert();	/* �ȴ�ת������ */
	UT = BMP180_Read2Bytes(0xF6);

	/* ��ѹ��ԭʼֵ */
	BMP180_WriteReg(0xF4, 0x34 + (g_tBMP180.OSS << 6));
	BMP180_WaitConvert();	/* �ȴ�ת������ */
	UP = BMP180_Read3Bytes(0xF6) >> (8 - g_tBMP180.OSS);

	/* ������ʵ�¶ȣ���λ 0.1���϶ȣ� */
	X1 = ((long)(UT - g_tBMP180.AC6) * g_tBMP180.AC5) >> 15;
	X2 = ((long)g_tBMP180.MC << 11) / (X1 + g_tBMP180.MD);
	B5 = X1 + X2;	/* ��ϵ��������ѹ�����¶Ȳ������� */
	T = (B5 + 8) >> 4;
	g_tBMP180.Temp = T;		/* ��������������ȫ�ֱ��� */

	/* ������ʵѹ��ֵ����λ Pa�� */
	B6 = B5 - 4000;
	X1 = (g_tBMP180.B2 * (B6 * B6) >> 12) >> 11;
	X2 = (g_tBMP180.AC2 * B6) >> 11;
	X3 = X1 + X2;
	B3 = (((((long)g_tBMP180.AC1) * 4 + X3) << g_tBMP180.OSS) + 2) >> 2;

	X1 = (g_tBMP180.AC3 * B6) >> 13;
	X2 = (g_tBMP180.B1 * ((B6 * B6) >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;
	B4 = (g_tBMP180.AC4 * (unsigned long)(X3 + 32768)) >> 15;

	B7 = ((unsigned long)(UP - B3) * (50000 >> g_tBMP180.OSS));
	if (B7 < 0x80000000)
	{
		p = (B7 << 1) / B4;
	}
	else
	{
		p = (B7 / B4) << 1;
	}

	X1 = (p >> 8) * (p >> 8);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * p) >> 16;
 	p =  p + ((X1 + X2 + 3791) >> 4);

	g_tBMP180.Press = p;		/* ��������������ȫ�ֱ��� */
}





/* ����I2C�������ӵ�GPIO�˿�, �û�ֻ��Ҫ�޸�����4�д��뼴������ı�SCL��SDA������ */
#define BMP180_GPIO_PORT_I2C	GPIOB/* GPIO�˿� */
#define BMP180_RCC_I2C_PORT 	RCC_AHB1Periph_GPIOB		/* GPIO�˿�ʱ�� */
#define BMP180_I2C_SCL_PIN		GPIO_Pin_4			/* ���ӵ�SCLʱ���ߵ�GPIO */
#define BMP180_I2C_SDA_PIN		GPIO_Pin_5			/* ���ӵ�SDA�����ߵ�GPIO */

/* �����дSCL��SDA�ĺ� */
#define BMP180_I2C_SCL_1()  BMP180_GPIO_PORT_I2C->BSRRL = BMP180_I2C_SCL_PIN				/* SCL = 1 */
#define BMP180_I2C_SCL_0()  BMP180_GPIO_PORT_I2C->BSRRH = BMP180_I2C_SCL_PIN				/* SCL = 0 */

#define BMP180_I2C_SDA_1()  BMP180_GPIO_PORT_I2C->BSRRL = BMP180_I2C_SDA_PIN				/* SDA = 1 */
#define BMP180_I2C_SDA_0()  BMP180_GPIO_PORT_I2C->BSRRH = BMP180_I2C_SDA_PIN				/* SDA = 0 */

#define BMP180_I2C_SDA_READ()  ((BMP180_GPIO_PORT_I2C->IDR & BMP180_I2C_SDA_PIN) != 0)	/* ��SDA����״̬ */
#define BMP180_I2C_SCL_READ()  ((BMP180_GPIO_PORT_I2C->IDR & BMP180_I2C_SCL_PIN) != 0)	/* ��SCL����״̬ */

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitI2C
*	����˵��: ����I2C���ߵ�GPIO������ģ��IO�ķ�ʽʵ��
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void BMP180_InitI2C(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(BMP180_RCC_I2C_PORT, ENABLE);	/* ��GPIOʱ�� */

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* ��Ϊ����� */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		/* ��Ϊ��©ģʽ */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* ���������費ʹ�� */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO������ٶ� */

	GPIO_InitStructure.GPIO_Pin = BMP180_I2C_SCL_PIN | BMP180_I2C_SDA_PIN;
	GPIO_Init(BMP180_GPIO_PORT_I2C, &GPIO_InitStructure);

	/* ��һ��ֹͣ�ź�, ��λI2C�����ϵ������豸������ģʽ */
	BMP180_i2c_Stop();
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_Delay
*	����˵��: I2C����λ�ӳ٣����400KHz
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void BMP180_i2c_Delay(void)
{
	uint8_t i;

	/*��
		CPU��Ƶ168MHzʱ�����ڲ�Flash����, MDK���̲��Ż�����̨ʽʾ�����۲Ⲩ�Ρ�
		ѭ������Ϊ5ʱ��SCLƵ�� = 1.78MHz (����ʱ: 92ms, ��д������������ʾ����̽ͷ���ϾͶ�дʧ�ܡ�ʱ��ӽ��ٽ�)
		ѭ������Ϊ10ʱ��SCLƵ�� = 1.1MHz (����ʱ: 138ms, ���ٶ�: 118724B/s)
		ѭ������Ϊ30ʱ��SCLƵ�� = 440KHz�� SCL�ߵ�ƽʱ��1.0us��SCL�͵�ƽʱ��1.2us

		��������ѡ��2.2Kŷʱ��SCL������ʱ��Լ0.5us�����ѡ4.7Kŷ����������Լ1us

		ʵ��Ӧ��ѡ��400KHz���ҵ����ʼ���
	*/
	for (i = 0; i < 30; i++);
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_Start
*	����˵��: CPU����I2C���������ź�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void BMP180_i2c_Start(void)
{
	/* ��SCL�ߵ�ƽʱ��SDA����һ�������ر�ʾI2C���������ź� */
	BMP180_I2C_SDA_1();
	BMP180_I2C_SCL_1();
	BMP180_i2c_Delay();
	BMP180_I2C_SDA_0();
	BMP180_i2c_Delay();
	BMP180_I2C_SCL_0();
	BMP180_i2c_Delay();
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_Start
*	����˵��: CPU����I2C����ֹͣ�ź�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void BMP180_i2c_Stop(void)
{
	/* ��SCL�ߵ�ƽʱ��SDA����һ�������ر�ʾI2C����ֹͣ�ź� */
	BMP180_I2C_SDA_0();
	BMP180_I2C_SCL_1();
	BMP180_i2c_Delay();
	BMP180_I2C_SDA_1();
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_SendByte
*	����˵��: CPU��I2C�����豸����8bit����
*	��    ��:  _ucByte �� �ȴ����͵��ֽ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void BMP180_i2c_SendByte(uint8_t _ucByte)
{
	uint8_t i;

	/* �ȷ����ֽڵĸ�λbit7 */
	for (i = 0; i < 8; i++)
	{
		if (_ucByte & 0x80)
		{
			BMP180_I2C_SDA_1();
		}
		else
		{
			BMP180_I2C_SDA_0();
		}
		BMP180_i2c_Delay();
		BMP180_I2C_SCL_1();
		BMP180_i2c_Delay();
		BMP180_I2C_SCL_0();
		if (i == 7)
		{
			 BMP180_I2C_SDA_1(); // �ͷ�����
		}
		_ucByte <<= 1;	/* ����һ��bit */
		BMP180_i2c_Delay();
	}
}

/*
*********************************************************************************************************
*	�� �� ��: BMP180_i2c_ReadByte
*	����˵��: CPU��I2C�����豸��ȡ8bit����
*	��    ��:  ��
*	�� �� ֵ: ����������
*********************************************************************************************************
*/
uint8_t BMP180_i2c_ReadByte(void)
{
	uint8_t i;
	uint8_t value;

	/* ������1��bitΪ���ݵ�bit7 */
	value = 0;
	for (i = 0; i < 8; i++)
	{
		value <<= 1;
		BMP180_I2C_SCL_1();
		BMP180_i2c_Delay();
		if (BMP180_I2C_SDA_READ())
		{
			value++;
		}
		BMP180_I2C_SCL_0();
		BMP180_i2c_Delay();
	}
	return value;
}

/*
*********************************************************************************************************
*	�� �� ��: BMP180_i2c_WaitAck
*	����˵��: CPU����һ��ʱ�ӣ�����ȡ������ACKӦ���ź�
*	��    ��:  ��
*	�� �� ֵ: ����0��ʾ��ȷӦ��1��ʾ��������Ӧ
*********************************************************************************************************
*/
uint8_t BMP180_i2c_WaitAck(void)
{
	uint8_t re;

	BMP180_I2C_SDA_1();	/* CPU�ͷ�SDA���� */
	BMP180_i2c_Delay();
	BMP180_I2C_SCL_1();	/* CPU����SCL = 1, ��ʱ�����᷵��ACKӦ�� */
	BMP180_i2c_Delay();
	if (BMP180_I2C_SDA_READ())	/* CPU��ȡSDA����״̬ */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	BMP180_I2C_SCL_0();
	BMP180_i2c_Delay();
	return re;
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_Ack
*	����˵��: CPU����һ��ACK�ź�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void BMP180_i2c_Ack(void)
{
	BMP180_I2C_SDA_0();	/* CPU����SDA = 0 */
	BMP180_i2c_Delay();
	BMP180_I2C_SCL_1();	/* CPU����1��ʱ�� */
	BMP180_i2c_Delay();
	BMP180_I2C_SCL_0();
	BMP180_i2c_Delay();
	BMP180_I2C_SDA_1();	/* CPU�ͷ�SDA���� */
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_NAck
*	����˵��: CPU����1��NACK�ź�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void BMP180_i2c_NAck(void)
{
	BMP180_I2C_SDA_1();	/* CPU����SDA = 1 */
	BMP180_i2c_Delay();
	BMP180_I2C_SCL_1();	/* CPU����1��ʱ�� */
	BMP180_i2c_Delay();
	BMP180_I2C_SCL_0();
	BMP180_i2c_Delay();
}

/*
*********************************************************************************************************
*	�� �� ��: i2c_CheckDevice
*	����˵��: ���I2C�����豸��CPU�����豸��ַ��Ȼ���ȡ�豸Ӧ�����жϸ��豸�Ƿ����
*	��    ��:  _Address���豸��I2C���ߵ�ַ
*	�� �� ֵ: ����ֵ 0 ��ʾ��ȷ�� ����1��ʾδ̽�⵽
*********************************************************************************************************
*/
uint8_t BMP180_i2c_CheckDevice(uint8_t _Address)
{
	uint8_t ucAck;


	if (BMP180_I2C_SDA_READ() && BMP180_I2C_SCL_READ())
	{
		BMP180_i2c_Start();		/* ���������ź� */

		/* �����豸��ַ+��д����bit��0 = w�� 1 = r) bit7 �ȴ� */
		BMP180_i2c_SendByte(_Address | BMP180_I2C_WR);
		ucAck = BMP180_i2c_WaitAck();	/* ����豸��ACKӦ�� */

		BMP180_i2c_Stop();			/* ����ֹͣ�ź� */

		return ucAck;
	}
	return 1;	/* I2C�����쳣 */
}
