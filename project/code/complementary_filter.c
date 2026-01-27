#include "complementary_filter.h"
#include <stddef.h>

// acc结构体（使用float保持精度）
typedef struct {
    float AX;
	float AY;
	float AZ;
}acc_xyz_value;

// gyro结构体（使用float保持精度）
typedef struct {
    float GX;
	float GY;
	float GZ;
}gyro_xyz_value;

extern acc_xyz_value Acc_value;
extern gyro_xyz_value Gyro_value;

// 互补滤波器
ComplementaryFilter_t cf_filter;
float roll, pitch, yaw;                   // 融合后的欧拉角
const float CF_ALPHA = 0.055f;             // 加速度计权重
const float CF_DT = 0.005f;                // 采样周期（秒），5ms

/**
 * @brief 初始化互补滤波器
 * @param filter 滤波器指针
 * @param alpha 加速度计权重 (0.0-1.0, 建议0.02-0.1)
 * @param dt 时间步长（秒），例如0.01表示10ms
 */
void CF_Init()
{   
    cf_filter.roll = 0.0f;
    cf_filter.pitch = 0.0f;
    cf_filter.yaw = 0.0f;
    cf_filter.alpha = CF_ALPHA;      // 加速度计权重
    cf_filter.dt = CF_DT;            // 采样周期
}

/**
 * @brief 从加速度计计算Roll和Pitch角
 * @param ax, ay, az 加速度计数据
 * @param roll 输出滚转角（度）指针
 * @param pitch 输出俯仰角（度）指针
 */
void CF_GetAccAngle(float ax, float ay, float az, 
                    float *roll, float *pitch)
{
    if (roll == NULL || pitch == NULL) 
        return;
    
    // 计算重力加速度的模长
    float acc_magnitude = sqrtf(ax*ax + ay*ay + az*az);
    
    // 避免除以零
    if (acc_magnitude < 0.1f) {
        *roll = 0.0f;
        *pitch = 0.0f;
        return;
    }
    
    // 归一化加速度
    ax = ax / acc_magnitude;
    ay = ay / acc_magnitude;
    az = az / acc_magnitude;
    
    // 使用反正切函数计算角度
    // Roll = atan2(ay, az)
    *roll = atan2f(ay, az) * 180.0f / PI;
    
    // Pitch = atan2(-ax, sqrt(ay^2 + az^2))
    *pitch = atan2f(-ax, sqrtf(ay*ay + az*az)) * 180.0f / PI;
}

/**
 * @brief 标准化角度到-180~180度范围
 * @param angle 输入角度（度）
 * @return 标准化后的角度
 */
float CF_NormalizeAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

/**
 * @brief 更新互补滤波器
 * 互补滤波原理：
 * 使用加速度计获得姿态的长期平均值（低频特性好）
 * 使用陀螺仪获得姿态的实时变化（短期响应快）
 * 通过加权融合两者获得更稳定的姿态估计
 * 
 * @param filter 滤波器指针
 * @param ax, ay, az 加速度计原始数据（m/s^2）
 * @param gx, gy, gz 陀螺仪原始数据（度/秒）
 */
ComplementaryFilter_t CF_Update()
{
    
    float acc_roll, acc_pitch;
    
    // 1. 从加速度计计算Roll和Pitch
    CF_GetAccAngle(Acc_value.AX, Acc_value.AY, Acc_value.AZ, &acc_roll, &acc_pitch);
    
//    // 2. 陀螺仪零偏补偿（可选，如需要可在标定时设置）
//	float gyro_x_corrected = gx - filter->gyro_bias_x;
//	float gyro_y_corrected = gy - filter->gyro_bias_y;
//	float gyro_z_corrected = gz - filter->gyro_bias_z;
    
    // 3. 陀螺仪积分获得增量（先进行归一化积分）
    // 使用更新前的值进行积分（保持连续性）
    float delta_roll = Gyro_value.GX * cf_filter.dt;
    float delta_pitch = Gyro_value.GY * cf_filter.dt;
    float delta_yaw = Gyro_value.GZ * cf_filter.dt;
    
    // 4. 互补滤波融合
    // 推荐的融合方式：加速度计用于纠正陀螺仪漂移，陀螺仪提供快速响应
    // Roll: 大部分来自陀螺仪积分（响应快），小部分从加速度计纠正（抗漂移）
    // Pitch: 同Roll原理
    // Yaw: 仅陀螺仪（加速度计无法获得Yaw）
    
    cf_filter.roll = (1.0f - cf_filter.alpha) * (cf_filter.roll + delta_roll) + cf_filter.alpha * acc_roll;
    cf_filter.pitch = (1.0f - cf_filter.alpha) * (cf_filter.pitch + delta_pitch) + cf_filter.alpha * acc_pitch;
    cf_filter.yaw = cf_filter.yaw + delta_yaw;  // Yaw只使用陀螺仪，因为加速度计无法获得Yaw信息
    
    // 5. 角度标准化（防止角度漂移过大）
    cf_filter.roll = CF_NormalizeAngle(cf_filter.roll);
    cf_filter.pitch = CF_NormalizeAngle(cf_filter.pitch);
    cf_filter.yaw = CF_NormalizeAngle(cf_filter.yaw);

    return  cf_filter;
}
