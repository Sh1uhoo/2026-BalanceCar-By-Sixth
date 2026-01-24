#include "zf_common_headfile.h"


uint8_t MPU6050_Init(void)
{
	uint8_t i=mpu6050_init();//初始化I2C，失败返回1
	return i;
}

uint8_t MPU6050_ReadWhoAmI(void)
{
	return (uint8_t)mpu6050_read_who_am_i();
}

void MPU6050_GetData_Acc(int16_t *ax,int16_t *ay,int16_t *az)
{
	mpu6050_get_acc();
	*ax=(int16_t)mpu6050_acc_transition(mpu6050_acc_x);//转换为物理数据x轴（float）
	*ay=(int16_t)mpu6050_acc_transition(mpu6050_acc_y);//转换为物理数据y轴（float）
	*az=(int16_t)mpu6050_acc_transition(mpu6050_acc_z);//转换为物理数据z轴（float）
}

void MPU6050_GetData_Gyro(int16_t *gx,int16_t *gy,int16_t *gz)
{
	mpu6050_get_gyro();
	*gx=(int16_t)mpu6050_gyro_transition(mpu6050_gyro_x);//转换为物理数据x（float）
	*gy=(int16_t)mpu6050_gyro_transition(mpu6050_gyro_y);//转换为物理数据y（float）
	*gz=(int16_t)mpu6050_gyro_transition(mpu6050_gyro_z);//转换为物理数据z（float）
}


