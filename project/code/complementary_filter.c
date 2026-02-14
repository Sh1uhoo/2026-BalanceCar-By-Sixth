#include "complementary_filter.h"
#include <stddef.h>

// acc结构体
typedef struct {
    float AX;
	float AY;
	float AZ;
}acc_xyz_value;

// gyro结构体
typedef struct {
    float GX;
	float GY;
	float GZ;
}gyro_xyz_value;

extern acc_xyz_value Acc_value;
extern gyro_xyz_value Gyro_value;

// 互补滤波器
ComplementaryFilter_t cf_filter;
float roll, pitch, yaw;                   // 融合后的欧拉角（使用float保持精度）
const float CF_ALPHA = 0.1f;              // 加速度计权重
const float CF_DT = 0.005f;                // 采样周期（秒），5ms


void CF_Init()
{   
    cf_filter.roll = 0.0f;
    cf_filter.pitch = 0.0f;
    cf_filter.yaw = 0.0f;
    cf_filter.alpha = CF_ALPHA;      // 加速度计权重
    cf_filter.dt = CF_DT;            // 采样周期
}

void CF_GetAccAngle(float ax, float ay, float az, 
                    float *roll, float *pitch)
{
    if (roll == NULL || pitch == NULL) 
        return;
    
    float acc_magnitude = sqrtf(ax*ax + ay*ay + az*az);
    
    if (acc_magnitude < 0.1f) {
        *roll = 0.0f;
        *pitch = 0.0f;
        return;
    }
    
    ax = ax / acc_magnitude;
    ay = ay / acc_magnitude;
    az = az / acc_magnitude;
    
    *roll = atan2f(ay, az) * 180.0f / PI;
    
    *pitch = atan2f(-ax, sqrtf(ay*ay + az*az)) * 180.0f / PI;
}


float CF_NormalizeAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

ComplementaryFilter_t CF_Update()
{
    
    float acc_roll, acc_pitch;
    
    CF_GetAccAngle(Acc_value.AX, Acc_value.AY, Acc_value.AZ, &acc_roll, &acc_pitch);
    
    float delta_roll = Gyro_value.GX * cf_filter.dt;
    float delta_pitch = Gyro_value.GY * cf_filter.dt;
    float delta_yaw = Gyro_value.GZ * cf_filter.dt;
    
    float gyro_roll = cf_filter.roll + delta_roll;
    float gyro_pitch = cf_filter.pitch + delta_pitch;
    float gyro_yaw = cf_filter.yaw + delta_yaw;
    
    float error_roll = acc_roll - gyro_roll;
    float error_pitch = acc_pitch - gyro_pitch;
    
    float max_error = 45.0f;  // 45度误差限制
    if (error_roll > max_error) error_roll = max_error;
    if (error_roll < -max_error) error_roll = -max_error;
    if (error_pitch > max_error) error_pitch = max_error;
    if (error_pitch < -max_error) error_pitch = -max_error;

    float adaptive_alpha = cf_filter.alpha;
    
    cf_filter.roll = gyro_roll + adaptive_alpha * error_roll;
    cf_filter.pitch = gyro_pitch + adaptive_alpha * error_pitch;
    cf_filter.yaw = gyro_yaw; 
    
    cf_filter.roll = CF_NormalizeAngle(cf_filter.roll);
    cf_filter.pitch = CF_NormalizeAngle(cf_filter.pitch);
    cf_filter.yaw = CF_NormalizeAngle(cf_filter.yaw);
	
	return cf_filter;
}
