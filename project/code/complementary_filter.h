#ifndef __COMPLEMENTARY_FILTER_H__
#define __COMPLEMENTARY_FILTER_H__

#include <stdint.h>
#include <math.h>

#ifndef PI
#define PI 3.14159265358979f
#endif

// 互补滤波器结构体
typedef struct {
    float roll;           // 滚转角（围绕X轴）
    float pitch;          // 俯仰角（围绕Y轴）
    float yaw;            // 偏航角（围绕Z轴）
    float alpha;          // 滤波系数（加速度计权重）
    float dt;             // 时间步长（秒）
} ComplementaryFilter_t;

/**
 * @brief 初始化互补滤波器
 * @param filter 滤波器指针
 * @param alpha 加速度计权重 (0.0-1.0, 建议0.02-0.1)
 * @param dt 时间步长（秒），例如0.01表示10ms
 */
void CF_Init();

/**
 * @brief 更新互补滤波器
 * @param filter 滤波器指针
 * @param ax, ay, az 加速度计原始数据
 * @param gx, gy, gz 陀螺仪原始数据（单位：度/秒）
 */
 ComplementaryFilter_t CF_Update();

/**
 * @brief 从加速度计计算Roll和Pitch角
 * @param ax, ay, az 加速度计数据
 * @param roll 输出滚转角（度）
 * @param pitch 输出俯仰角（度）
 */
void CF_GetAccAngle(float ax, float ay, float az, 
                    float *roll, float *pitch);

/**
 * @brief 标准化角度到-180~180度范围
 * @param angle 输入角度（度）
 * @return 标准化后的角度
 */
float CF_NormalizeAngle(float angle);

#endif // __COMPLEMENTARY_FILTER_H__
