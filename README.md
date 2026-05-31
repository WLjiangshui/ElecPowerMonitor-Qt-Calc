# 电力负荷计算与监测系统

基于 Qt6 + CMake 的三相/单相电力参数计算工具，适用于电气工程学习与仿真。

## 功能
- 星形(Y) / 三角形(Δ) / 单相 接线计算
- 视在功率、有功功率、无功功率、功率因数
- 功率因数评价
- 已知功率反推电流

## 构建
```bash
cmake -B build -G "Ninja"
cmake --build build
```

## 技术栈
- Qt 6.5+ (Widgets + Charts)
- C++17
