#ifndef __MPU6050_H
#define __MPU6050_H

// acc结构体（使用float保持精度）
typedef struct {
    float AX;
	float AY;
	float AZ;
} acc_xyz_value;

// gyro结构体（使用float保持精度）
typedef struct {
    float GX;
	float GY;
	float GZ;
} gyro_xyz_value;


typedef struct {
	float AX_0;
	float AY_0;
	float AZ_0;
    float GX_0;
	float GY_0;
	float GZ_0;
} check;

uint8_t MPU6050_Init(void);
void MPU6050_GetData_Acc();
void MPU6050_GetData_Gyro();
void MPU6050_GetData();
void Check_Init();
void MPU6050_Check();

#endif
