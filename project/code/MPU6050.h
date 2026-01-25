#ifndef __MPU6050_H
#define __MPU6050_H

// acc结构体
typedef struct {
    int16_t AX;
	int16_t AY;
	int16_t	AZ;
} acc_xyz_value;

// gyro结构体
typedef struct {
    int16_t GX;
	int16_t GY;
	int16_t	GZ;
} gyro_xyz_value;


uint8_t MPU6050_Init(void);
void MPU6050_GetData_Acc(acc_xyz_value *Acc);
void MPU6050_GetData_Gyro(gyro_xyz_value *Gyro);
void MPU6050_GetData(acc_xyz_value *Acc,gyro_xyz_value *Gyro);

#endif
