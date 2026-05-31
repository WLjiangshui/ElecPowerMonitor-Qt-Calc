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

THDResult PowerCalculator::calculateTHD(const std::vector<double> &harmonics)
{
    THDResult result;
    if (harmonics.empty()) return result;

    double fundamental = harmonics[0];  // 基波幅值
    if (fundamental == 0.0) return result;

    double sumSquares = 0.0;
    for (size_t i = 1; i < harmonics.size(); ++i) {
        sumSquares += harmonics[i] * harmonics[i];
    }

    double thd = (std::sqrt(sumSquares) / fundamental) * 100.0;
    result.thdU = thd;
    result.thdI = thd;
    result.passed = isTHDPass(result.thdU, result.thdI);

    return result;
}

bool PowerCalculator::isTHDPass(double thdU, double thdI, double voltageLevel)
{
    // GB/T 14549-93 谐波电压限值
    double thdULimit = 5.0;  // 380V及以下：THDu ≤ 5%
    if (voltageLevel > 380 && voltageLevel <= 1000) {
        thdULimit = 4.0;
    } else if (voltageLevel > 1000) {
        thdULimit = 3.0;
    }

    // 电流谐波限值（简化：参考IEC 61000-3-2，一般≤20%）
    double thdILimit = 20.0;

    return (thdU <= thdULimit) && (thdI <= thdILimit);
}
