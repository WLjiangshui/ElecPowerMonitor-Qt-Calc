#include "powercalculator.h"

PowerCalculator::PowerCalculator(ConnectionType type)
    : connType_(type)
{
}

PowerResult PowerCalculator::calculate() const
{
    PowerResult result;
    const double sqrt3 = std::sqrt(3.0);

    // 计算视在功率 S
    if (connType_ == Star || connType_ == Delta) {
        result.apparentPower = sqrt3 * voltage_ * current_;
    } else {  // 单相
        result.apparentPower = voltage_ * current_;
    }

    // 有功功率 P = S × cosφ
    result.activePower = result.apparentPower * powerFactor_;

    // 无功功率 Q = S × sinφ = S × √(1-cos²φ)
    double sinPhi = std::sqrt(1.0 - powerFactor_ * powerFactor_);
    result.reactivePower = result.apparentPower * sinPhi;

    return result;
}
