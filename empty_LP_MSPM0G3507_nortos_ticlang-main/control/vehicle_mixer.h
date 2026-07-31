#ifndef VEHICLE_MIXER_H
#define VEHICLE_MIXER_H

/**
 * @file vehicle_mixer.h
 * @brief 把前进速度和转向速度差转换为左右轮目标速度。
 */

/** 左右轮目标，单位 mm/s。 */
typedef struct {
    float leftMmS;
    float rightMmS;
} WheelSpeed_Targets;

/**
 * @brief 差速混合并按比例限制最大轮速。
 * @param steeringMmS 正值表示右转，即左轮加速、右轮减速。
 */
WheelSpeed_Targets VehicleMixer_Mix(float forwardMmS, float steeringMmS,
                                    float wheelSpeedLimitMmS);

#endif
