#include "complementary_filter.h"
#include <stddef.h>

/**
 * @brief 初始化互补滤波器
 * @param filter 滤波器指针
 * @param alpha 加速度计权重 (0.0-1.0, 建议0.02-0.1)
 * @param dt 时间步长（秒），例如0.01表示10ms
 */
void CF_Init(ComplementaryFilter_t *filter, float alpha, float dt)
{
    if (filter == NULL) return;
    
    filter->roll = 0.0f;
    filter->pitch = 0.0f;
    filter->yaw = 0.0f;
    filter->alpha = alpha;      // 加速度计权重，通常设置为0.02-0.1
    filter->dt = dt;            // 采样周期
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
 * @param ax, ay, az 加速度计原始数据
 * @param gx, gy, gz 陀螺仪原始数据（单位：度/秒）
 */
void CF_Update(ComplementaryFilter_t *filter, 
               float ax, float ay, float az,
               float gx, float gy, float gz)
{
    if (filter == NULL) return;
    
    float acc_roll, acc_pitch;
    
    // 1. 从加速度计计算Roll和Pitch
    CF_GetAccAngle(ax, ay, az, &acc_roll, &acc_pitch);
    
    // 2. 陀螺仪积分获得角速度变化
    float gyro_roll = filter->roll + gx * filter->dt;
    float gyro_pitch = filter->pitch + gy * filter->dt;
    float gyro_yaw = filter->yaw + gz * filter->dt;
    
    // 3. 互补滤波融合
    // 加速度计权重为alpha，陀螺仪权重为(1-alpha)
    // alpha值越大，加速度计的影响越大；alpha值越小，陀螺仪的影响越大
    filter->roll = (1.0f - filter->alpha) * gyro_roll + filter->alpha * acc_roll;
    filter->pitch = (1.0f - filter->alpha) * gyro_pitch + filter->alpha * acc_pitch;
    filter->yaw = gyro_yaw;  // Yaw只使用陀螺仪，因为加速度计无法获得Yaw信息
    
    // 4. 角度标准化（防止角度漂移过大）
    filter->roll = CF_NormalizeAngle(filter->roll);
    filter->pitch = CF_NormalizeAngle(filter->pitch);
    filter->yaw = CF_NormalizeAngle(filter->yaw);
}
