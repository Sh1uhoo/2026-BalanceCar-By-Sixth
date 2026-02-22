#include "navigation.h"
#include "zf_driver_flash.h"
#include <math.h>

#define DEG_TO_RAD (3.1415926535897932384626 / 180.0)
#define GRAVITY 9.8f // 重力加速度，单位 m/s^2

typedef struct {
  float x, y, z;
} vec3f;
typedef struct {
  float w, x, y, z;
} quatf;

extern vec3f Acc_value;  // deg/s 单位
extern vec3f Gyro_value; // g 单位，g取9.8m/s^2

// 向量点积
float vec_dot(vec3f a, vec3f b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

// 向量叉积
vec3f vec_cross(vec3f a, vec3f b) {
  vec3f result;
  result.x = a.y * b.z - a.z * b.y;
  result.y = a.z * b.x - a.x * b.z;
  result.z = a.x * b.y - a.y * b.x;
  return result;
}

// 向量长度（模）
float vec_length(vec3f v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }
void vec_normalize(vec3f *v) {
  float len_inv = 1 / vec_length(*v);
  v->x *= len_inv;
  v->y *= len_inv;
  v->z *= len_inv;
}
void vec_smul(vec3f *v, float k) {
  v->x *= k;
  v->y *= k;
  v->z *= k;
}
void vec_add(vec3f *v, vec3f w) {
  v->x += w.x;
  v->y += w.y;
  v->z += w.z;
}
vec3f vec_sclamp(vec3f v, float m, float M) {
  if (v.x < m)
    v.x = m;
  if (v.x > M)
    v.x = M;
  if (v.y < m)
    v.y = m;
  if (v.y > M)
    v.y = M;
  if (v.z < m)
    v.z = m;
  if (v.z > M)
    v.z = M;
  return v;
}

// 四元数乘法：res = q1 * q2
quatf quat_multiply(quatf q1, quatf q2) {
  quatf res;
  res.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
  res.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
  res.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
  res.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
  return res;
}

// 用四元数旋转向量：v_out = q * v * q_conj
vec3f quat_rotate(quatf q, vec3f v) {
  float x = q.x, y = q.y, z = q.z, w = q.w;
  float vx = v.x, vy = v.y, vz = v.z;
  // 直接使用旋转矩阵公式
  float xx = x * x, yy = y * y, zz = z * z;
  float xy = x * y, xz = x * z, yz = y * z;
  float wx = w * x, wy = w * y, wz = w * z;

  vec3f res;
  res.x = vx * (1 - 2 * (yy + zz)) + vy * 2 * (xy - wz) + vz * 2 * (xz + wy);
  res.y = vx * 2 * (xy + wz) + vy * (1 - 2 * (xx + zz)) + vz * 2 * (yz - wx);
  res.z = vx * 2 * (xz - wy) + vy * 2 * (yz + wx) + vz * (1 - 2 * (xx + yy));
  return res;
}

// 归一化四元数
void quat_normalize(quatf *q) {
  float norm = sqrtf(q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);
  if (norm > 1e-6f) {
    q->w /= norm;
    q->x /= norm;
    q->y /= norm;
    q->z /= norm;
  }
}

// 取四元数共轭
void quat_conj(quatf *q) {
  q->x = -q->x;
  q->y = -q->y;
  q->z = -q->z;
}

typedef struct {
  float ki, kp;
  float clamp;
  vec3f error_int;
  quatf q;
  vec3f vel, pos;
} PoseInfo;

// 迭代函数：更新姿态、速度、位置
void update_pose(PoseInfo *pose, vec3f gyro, vec3f acc, float dt) {
  float gx = gyro.x * DEG_TO_RAD;
  float gy = gyro.y * DEG_TO_RAD;
  float gz = gyro.z * DEG_TO_RAD;

  int is_falling = 1;

  // 1. 互补滤波
  // 归一化加速度计测量值
  float acc_norm = vec_length(acc);
  if (0.2f < acc_norm && acc_norm < 1.5f) {
    is_falling = 0;
    vec_smul(&acc, 1 / acc_norm);

    vec3f gravity_gyro; // 陀螺仪估计的重力方向
    {
      float w = pose->q.w, x = pose->q.x, y = pose->q.y, z = pose->q.z;
      gravity_gyro.x = 2 * (x * z - w * y);
      gravity_gyro.y = 2 * (y * z + w * x);
      gravity_gyro.z = w * w - x * x - y * y + z * z;
      vec_normalize(&gravity_gyro);
    }

    // 计算误差：加速度计测量值与估计值的叉积（载体坐标系）
    vec3f error, error_dt;
    error = vec_cross(acc, gravity_gyro);
    error_dt = error;

    // 积分误差
    vec_smul(&error_dt, dt);
    vec_add(&pose->error_int, error_dt);
    // 积分限幅(也可以使用模长作为标准)
    pose->error_int = vec_sclamp(pose->error_int, -pose->clamp, pose->clamp);

    // 应用补偿：补偿量 = KP * error + KI * pose->error_int
    gx += pose->kp * error.x + pose->ki * pose->error_int.x;
    gy += pose->kp * error.y + pose->ki * pose->error_int.y;
    gz += pose->kp * error.z + pose->ki * pose->error_int.z;
  }

  // 2. 用补偿后的陀螺仪更新四元数
  // 计算角增量
  float delta_x = gx * dt;
  float delta_y = gy * dt;
  float delta_z = gz * dt;
  float theta =
      sqrtf(delta_x * delta_x + delta_y * delta_y + delta_z * delta_z);

  // 角度增量转四元数
  quatf q_delta;
  if (theta > 1e-10f) {
    float half = theta * 0.5f;
    float sin_half = sinf(half) / theta;
    q_delta.w = cosf(half);
    q_delta.x = delta_x * sin_half;
    q_delta.y = delta_y * sin_half;
    q_delta.z = delta_z * sin_half;
  } else {
    q_delta.w = 1.0f;
    q_delta.x = q_delta.y = q_delta.z = 0.0f;
  }

  // 3. 速度、位置积分
  // 转换单位：g -> m/s^2
  vec3f acc_body;
  acc_body.x = acc.x * GRAVITY;
  acc_body.y = acc.y * GRAVITY;
  acc_body.z = acc.z * GRAVITY;

  // 转换坐标系（使用东北天坐标，虽然没有磁力计校准方向）
  vec3f acc_nav = quat_rotate(pose->q, acc_body);

  // 去除重力影响
  vec3f a;
  a.x = acc_nav.x;
  a.y = acc_nav.y;
  a.z = acc_nav.z - (is_falling ? 0.0f : GRAVITY);

  vec3f v_new;
  v_new.x = pose->vel.x + a.x * dt;
  v_new.y = pose->vel.y + a.y * dt;
  v_new.z = pose->vel.z + a.z * dt;

  pose->pos.x += (pose->vel.x + v_new.x) * 0.5f * dt;
  pose->pos.y += (pose->vel.y + v_new.y) * 0.5f * dt;
  pose->pos.z += (pose->vel.z + v_new.z) * 0.5f * dt;

  pose->vel = v_new;
}

#define MAX_RECORD_POINT 1360
typedef struct {
  vec3f points[MAX_RECORD_POINT];
  int current_num;
} PathRecord;

void NAV_PathInit(PathRecord *self) { self->current_num = 0; }
void NAV_PathInsert(PathRecord *self, vec3f new_point) {
  if (self->current_num < MAX_RECORD_POINT) {
    self->points[self->current_num++] = new_point;
  } else {
  }
}
void NAV_PathSqueeze(PathRecord *self, int k) {
#define MIN(x, y) ((x) < (y) ? (x) : (y))
  if (self->current_num <= 2 || k <= 1)
    return;
  int cur = 1, last = 1;
  for (int lower = 1, upper = MIN(self->current_num, lower + k);
       lower < self->current_num;
       lower += k, upper = MIN(self->current_num, lower + k)) {
    last++;
    for (int l = lower + 1; l < upper; ++l)
      self->points[cur++] = self->points[last++];
  }
  self->current_num -= last - cur;
#undef MIN
}
void NAV_SavePath(PathRecord *self) {
  // 126-127扇区被占用，使用122-125扇区，最大16KB=>4K个float/i32
  // 每页85个点
  int num = self->current_num;
  int i, sector, page;
  for (i = 0; i < num / 85; ++i) {
    flash_buffer_clear();
    flash_union_buffer[255].int32_type = num;
    for (int j = 0; j < 85; ++j) {
      flash_union_buffer[j * 3 + 0].float_type = self->points[i * 85 + j].x;
      flash_union_buffer[j * 3 + 1].float_type = self->points[i * 85 + j].y;
      flash_union_buffer[j * 3 + 2].float_type = self->points[i * 85 + j].z;
    }
    sector = 122 + i / 4, page = i % 4;
    if (sector > 125)
      return;
    flash_write_page_from_buffer(sector, page);
  }
  flash_buffer_clear();
  flash_union_buffer[255].int32_type = num;
  for (int j = 0; j < num % 85; ++j) {
    flash_union_buffer[j * 3 + 0].float_type = self->points[i * 85 + j].x;
    flash_union_buffer[j * 3 + 1].float_type = self->points[i * 85 + j].y;
    flash_union_buffer[j * 3 + 2].float_type = self->points[i * 85 + j].z;
  }
  sector = 122 + i / 4, page = i % 4;
  if (sector > 125)
    return;
  flash_write_page_from_buffer(sector, page);
}
