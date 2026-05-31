#ifndef POWERCALCULATOR_H
#define POWERCALCULATOR_H

#include <cmath>
#include <vector>

struct PowerResult {
    double apparentPower = 0.0;   // 视在功率 S (VA)
    double activePower = 0.0;     // 有功功率 P (W)
    double reactivePower = 0.0;   // 无功功率 Q (var)
};

struct THDResult {
    double thdU = 0.0;     // 电压总谐波畸变率 THDu (%)
    double thdI = 0.0;     // 电流总谐波畸变率 THDi (%)
    bool passed = false;   // 是否满足国标限值
};

struct CostResult {
    double dailyKWh = 0.0;      // 日用电量 (kWh)
    double monthlyKWh = 0.0;    // 月用电量 (kWh)
    double dailyCost = 0.0;     // 日电费 (元)
    double monthlyCost = 0.0;   // 月电费 (元)
    int tier = 0;               // 所处阶梯档位
};

class PowerCalculator
{
public:
    enum ConnectionType {
        Star,          // 星形接法：S = √3 × U_L × I_L
        Delta,         // 三角形接法：S = √3 × U_L × I_L
        SinglePhase    // 单相：S = U × I
    };

    explicit PowerCalculator(ConnectionType type = Star);

    void setConnectionType(ConnectionType type) { connType_ = type; }
    void setVoltage(double U) { voltage_ = U; }
    void setCurrent(double I) { current_ = I; }
    void setPowerFactor(double pf) { powerFactor_ = pf; }

    PowerResult calculate() const;

    // 谐波计算：输入各次谐波幅值（基波在索引0），返回THD结果
    static THDResult calculateTHD(const std::vector<double> &harmonics);
    // 根据国标GB/T 14549判断THD是否合格
    static bool isTHDPass(double thdU, double thdI, double voltageLevel = 380);

    // 电费计算：三级阶梯电价（居民用电标准）
    static CostResult calculateCost(double powerKW, double hoursPerDay = 24.0);
    // 评估负载率
    static QString evaluateLoadRate(double actualKw, double ratedKw);

private:
    ConnectionType connType_;
    double voltage_ = 0.0;
    double current_ = 0.0;
    double powerFactor_ = 1.0;
};

#endif // POWERCALCULATOR_H
