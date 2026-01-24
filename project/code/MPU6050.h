#ifndef __MPU6050_H
#define __MPU6050_H


uint8_t MPU6050_Init(void);
void MPU6050_GetData_Acc(int16_t *ax,int16_t *ay,int16_t *az);
void MPU6050_GetData_Gyro(int16_t *gx,int16_t *gy,int16_t *gz);
uint8_t MPU6050_ReadWhoAmI(void);

#endif
