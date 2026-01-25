#include "zf_common_headfile.h"
#include "MPU6050.h"

uint8_t MPU6050_Init(void)
{
	uint8_t i=mpu6050_init();//初始化I2C，失败返回1
	return i;
}


// MPU6500的配置：在 zf_device_mpu6050.h 文件 MPU6050_ACC_SAMPLE(102)  MPU6050_GYR_SAMPLE(108)  中查看
void MPU6050_GetData_Acc(acc_xyz_value *Acc,check *ch)
{
	mpu6050_get_acc();
	Acc->AX = mpu6050_acc_transition(mpu6050_acc_x)-ch->AX_0;
	Acc->AY = mpu6050_acc_transition(mpu6050_acc_y)-ch->AY_0;
	Acc->AZ = mpu6050_acc_transition(mpu6050_acc_z)-(ch->AZ_0-1);
}

void MPU6050_GetData_Gyro(gyro_xyz_value *Gyro,check *ch)
{
	mpu6050_get_gyro();
	Gyro->GX = mpu6050_gyro_transition(mpu6050_gyro_x)-ch->GX_0;
	Gyro->GY = mpu6050_gyro_transition(mpu6050_gyro_y)-ch->GY_0;
	Gyro->GZ = mpu6050_gyro_transition(mpu6050_gyro_z)-ch->GZ_0;
}

void MPU6050_GetData(acc_xyz_value *Acc,gyro_xyz_value *Gyro,check *ch)
{
	MPU6050_GetData_Acc(Acc,ch);
	MPU6050_GetData_Gyro(Gyro,ch);
}

void Check_Init(check *ch)
{
    if (ch == NULL) return;
    ch->AX_0 = 0.0f;
    ch->AY_0 = 0.0f;
    ch->AZ_0 = 0.0f;
    ch->GX_0 = 0.0f;
	ch->GY_0 = 0.0f;
	ch->GZ_0 = 0.0f;
}

void MPU6050_Check(check *ch)
{
	Check_Init(ch);
	system_delay_ms(100);
	mpu6050_get_acc();
	mpu6050_get_gyro();
	ch->AX_0 = mpu6050_acc_transition(mpu6050_acc_x);
	ch->AY_0 = mpu6050_acc_transition(mpu6050_acc_y);
	ch->AZ_0 = mpu6050_acc_transition(mpu6050_acc_z);
	ch->GX_0 = mpu6050_gyro_transition(mpu6050_gyro_x); 
	ch->GY_0 = mpu6050_gyro_transition(mpu6050_gyro_y);
	ch->GZ_0 = mpu6050_gyro_transition(mpu6050_gyro_z);
	
}
