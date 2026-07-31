/*
 * 双轮差速混合器。
 *
 * 与直接分别裁剪左右轮相比，超限时统一按比例缩放可以保留左右速度比，从而尽量
 * 保持原转弯半径。
 */
#include "control/vehicle_mixer.h"

static float absolute(float value) { return (value < 0.0f) ? -value : value; }

WheelSpeed_Targets VehicleMixer_Mix(float forwardMmS, float steeringMmS,
                                    float wheelSpeedLimitMmS)
{
    WheelSpeed_Targets result;
    float largest;
    float scale;

    /* 正转向量表示右转：左轮提高、右轮降低。 */
    result.leftMmS = forwardMmS + steeringMmS;
    result.rightMmS = forwardMmS - steeringMmS;
    largest = absolute(result.leftMmS);
    if (absolute(result.rightMmS) > largest) largest = absolute(result.rightMmS);
    if ((largest > wheelSpeedLimitMmS) && (largest > 0.0f)) {
        /* 两侧应用相同比例，保留方向控制器给出的差速关系。 */
        scale = wheelSpeedLimitMmS / largest;
        result.leftMmS *= scale;
        result.rightMmS *= scale;
    }
    return result;
}
