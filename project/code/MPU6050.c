#include "zf_common_headfile.h"
#include "MPU6050.h"

uint8_t MPU6050_Init(void)
{
	uint8_t i=mpu6050_init();//初始化I2C，失败返回1
	return i;
}


// MPU6500的配置：在 zf_device_mpu6050.h 文件 MPU6050_ACC_SAMPLE(102)  MPU6050_GYR_SAMPLE(108)  中查看
void MPU6050_GetData_Acc(acc_xyz_value *Acc)
{
	mpu6050_get_acc();
	Acc->AX=(int16_t)mpu6050_acc_transition(mpu6050_acc_x);//转换为物理数据x轴（float）
	Acc->AY=(int16_t)mpu6050_acc_transition(mpu6050_acc_y);//转换为物理数据y轴（float）
	Acc->AZ=(int16_t)mpu6050_acc_transition(mpu6050_acc_z);//转换为物理数据z轴（float）
}

void MPU6050_GetData_Gyro(gyro_xyz_value *Gyro)
{
	mpu6050_get_gyro();
	Gyro->GX=(int16_t)mpu6050_gyro_transition(mpu6050_gyro_x);//转换为物理数据x（float）
	Gyro->GY=(int16_t)mpu6050_gyro_transition(mpu6050_gyro_y);//转换为物理数据y（float）
	Gyro->GZ=(int16_t)mpu6050_gyro_transition(mpu6050_gyro_z);//转换为物理数据z（float）
}

void MPU6050_GetData(acc_xyz_value *Acc,gyro_xyz_value *Gyro)
{
	MPU6050_GetData_Acc(Acc);
	MPU6050_GetData_Gyro(Gyro);
}
