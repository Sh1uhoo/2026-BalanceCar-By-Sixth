#include "zf_common_headfile.h"
#include "MPU6050.h"

check check_0;
acc_xyz_value Acc_value;
gyro_xyz_value Gyro_value;				//结构体

uint8_t MPU6050_Init(void)
{
	uint8_t i=mpu6050_init();//初始化I2C，失败返回1
	MPU6050_Check();
	return i;
}


// MPU6500的配置：在 zf_device_mpu6050.h 文件 MPU6050_ACC_SAMPLE(102)  MPU6050_GYR_SAMPLE(108)  中查看
void MPU6050_GetData_Acc()
{
	mpu6050_get_acc();
	Acc_value.AX = mpu6050_acc_transition(mpu6050_acc_x)-check_0.AX_0;
	Acc_value.AY = mpu6050_acc_transition(mpu6050_acc_y)-check_0.AY_0;
	Acc_value.AZ = mpu6050_acc_transition(mpu6050_acc_z)-(check_0.AZ_0-1);
}

void MPU6050_GetData_Gyro()
{
	mpu6050_get_gyro();
	Gyro_value.GX = mpu6050_gyro_transition(mpu6050_gyro_x)-check_0.GX_0;
	Gyro_value.GY = mpu6050_gyro_transition(mpu6050_gyro_y)-check_0.GY_0;
	Gyro_value.GZ = mpu6050_gyro_transition(mpu6050_gyro_z)-check_0.GZ_0;
}

void MPU6050_GetData()
{
	MPU6050_GetData_Acc();
	MPU6050_GetData_Gyro();
}

void Check_Init()
{
    check_0.AX_0 = 0.0f;
    check_0.AY_0 = 0.0f;
    check_0.AZ_0 = 0.0f;
    check_0.GX_0 = 0.0f;
	check_0.GY_0 = 0.0f;
	check_0.GZ_0 = 0.0f;
}

void MPU6050_Check()
{
	Check_Init();
	for(int i=0;i<100;i++)
	{
	mpu6050_get_acc();
	mpu6050_get_gyro();
	check_0.AX_0 += mpu6050_acc_transition(mpu6050_acc_x);
	check_0.AY_0 += mpu6050_acc_transition(mpu6050_acc_y);
	check_0.AZ_0 += mpu6050_acc_transition(mpu6050_acc_z);
	check_0.GX_0 += mpu6050_gyro_transition(mpu6050_gyro_x); 
	check_0.GY_0 += mpu6050_gyro_transition(mpu6050_gyro_y);
	check_0.GZ_0 += mpu6050_gyro_transition(mpu6050_gyro_z);
	}
	check_0.AX_0 /=100;
	check_0.AY_0 /=100;
	check_0.AZ_0 /=100;
	check_0.GX_0 /=100;
	check_0.GY_0 /=100;
	check_0.GZ_0 /=100;
}
