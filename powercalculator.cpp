#include "powercalculator.h"

#include <QString>

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

CostResult PowerCalculator::calculateCost(double powerKW, double hoursPerDay)
{
    CostResult result;

    // 日用电量 = 功率(kW) × 日运行小时数
    result.dailyKWh = powerKW * hoursPerDay;
    // 月用电量（按30天计）
    result.monthlyKWh = result.dailyKWh * 30.0;

    // 三级阶梯电价（居民用电参考标准）
    // 第一档：0~210 kWh/月 → 0.53 元/kWh
    // 第二档：210~400 kWh/月 → 0.58 元/kWh
    // 第三档：>400 kWh/月 → 0.83 元/kWh
    double monthly = result.monthlyKWh;
    double cost = 0.0;

    if (monthly <= 210.0) {
        cost = monthly * 0.53;
        result.tier = 1;
    } else if (monthly <= 400.0) {
        cost = 210.0 * 0.53 + (monthly - 210.0) * 0.58;
        result.tier = 2;
    } else {
        cost = 210.0 * 0.53 + (400.0 - 210.0) * 0.58 + (monthly - 400.0) * 0.83;
        result.tier = 3;
    }

    result.monthlyCost = cost;
    result.dailyCost = cost / 30.0;

    return result;
}

QString PowerCalculator::evaluateLoadRate(double actualKw, double ratedKw)
{
    if (ratedKw <= 0) return "额定功率无效";

    double loadRate = (actualKw / ratedKw) * 100.0;

    if (loadRate < 30.0) {
        return QString("负载率 %1% - 轻载运行，效率偏低").arg(loadRate, 0, 'f', 1);
    } else if (loadRate < 70.0) {
        return QString("负载率 %1% - 经济运行区间").arg(loadRate, 0, 'f', 1);
    } else if (loadRate <= 100.0) {
        return QString("负载率 %1% - 接近满载，注意温升").arg(loadRate, 0, 'f', 1);
    } else {
        return QString("负载率 %1% - 过载运行！存在安全风险").arg(loadRate, 0, 'f', 1);
    }
}
