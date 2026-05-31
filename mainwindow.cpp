#include "mainwindow.h"
#include "powercalculator.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QFrame>
#include <QHeaderView>
#include <QMessageBox>
#include <vector>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

void MainWindow::setupUI()
{
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    tabWidget_ = new QTabWidget(this);

    // 功率计算选项卡
    auto *powerTab = new QWidget(this);
    setupPowerTab(powerTab);
    tabWidget_->addTab(powerTab, "⚡ 功率计算");

    // 谐波分析选项卡
    auto *thdTab = new QWidget(this);
    setupTHDTab(thdTab);
    tabWidget_->addTab(thdTab, "🔧 谐波分析 (THD)");

    // 电费估算 & 负载评估选项卡
    auto *costTab = new QWidget(this);
    setupCostTab(costTab);
    tabWidget_->addTab(costTab, "💰 电费 & 负载率");

    mainLayout->addWidget(tabWidget_);
}

void MainWindow::setupPowerTab(QWidget *tab)
{
    auto *mainLayout = new QVBoxLayout(tab);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ---- 接线方式选择 ----
    auto *connGroup = new QGroupBox("接线方式", tab);
    auto *connLayout = new QHBoxLayout(connGroup);
    comboConnection_ = new QComboBox(tab);
    comboConnection_->addItem("星形接法（Y） - 三相四线");
    comboConnection_->addItem("三角形接法（Δ） - 三相三线");
    comboConnection_->addItem("单相电路");
    comboConnection_->setCurrentIndex(0);
    connLayout->addWidget(comboConnection_);
    mainLayout->addWidget(connGroup);

    // ---- 参数输入 ----
    auto *inputGroup = new QGroupBox("输入参数", tab);
    auto *inputLayout = new QFormLayout(inputGroup);

    editVoltage_ = new QLineEdit(tab);
    editVoltage_->setPlaceholderText("如 380");
    editCurrent_ = new QLineEdit(tab);
    editCurrent_->setPlaceholderText("如 10");
    editPowerFactor_ = new QLineEdit(tab);
    editPowerFactor_->setPlaceholderText("0~1，如 0.85");

    inputLayout->addRow("线电压/相电压：", editVoltage_);
    inputLayout->addRow("线电流/相电流：", editCurrent_);
    inputLayout->addRow("功率因数 cosφ：", editPowerFactor_);

    mainLayout->addWidget(inputGroup);

    // ---- 按钮 ----
    auto *btnLayout = new QHBoxLayout();
    auto *btnCalc = new QPushButton("计算", tab);
    auto *btnClear = new QPushButton("清空", tab);
    btnCalc->setMinimumHeight(36);
    btnClear->setMinimumHeight(36);
    btnLayout->addWidget(btnCalc);
    btnLayout->addWidget(btnClear);
    mainLayout->addLayout(btnLayout);

    connect(btnCalc, &QPushButton::clicked, this, &MainWindow::onCalculate);
    connect(btnClear, &QPushButton::clicked, this, &MainWindow::onClear);

    // ---- 结果显示 ----
    auto *resultGroup = new QGroupBox("计算结果", tab);
    auto *resultLayout = new QVBoxLayout(resultGroup);
    textResult_ = new QTextEdit(tab);
    textResult_->setReadOnly(true);
    textResult_->setMinimumHeight(200);
    resultLayout->addWidget(textResult_);
    mainLayout->addWidget(resultGroup);
}

void MainWindow::setupTHDTab(QWidget *tab)
{
    auto *mainLayout = new QVBoxLayout(tab);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ---- 电压等级 ----
    auto *volGroup = new QGroupBox("系统参数", tab);
    auto *volLayout = new QFormLayout(volGroup);
    editVoltageTHD_ = new QLineEdit(tab);
    editVoltageTHD_->setPlaceholderText("如 380 V");
    editVoltageTHD_->setText("380");
    volLayout->addRow("额定电压 (V)：", editVoltageTHD_);
    mainLayout->addWidget(volGroup);

    // ---- 谐波输入表格 ----
    auto *harmGroup = new QGroupBox("各次谐波含有率 (%)", tab);
    auto *harmLayout = new QVBoxLayout(harmGroup);

    tableHarmonics_ = new QTableWidget(7, 2, tab);
    tableHarmonics_->setHorizontalHeaderLabels({"谐波次数 h", "含有率 (%)"});
    tableHarmonics_->horizontalHeader()->setStretchLastSection(true);
    tableHarmonics_->setAlternatingRowColors(true);

    // 预填常见奇次谐波
    QStringList harmonics = {"基波(1次)", "3次", "5次", "7次", "9次", "11次", "13次"};
    for (int i = 0; i < harmonics.size(); ++i) {
        tableHarmonics_->setItem(i, 0, new QTableWidgetItem(harmonics[i]));
        auto *valueItem = new QTableWidgetItem(i == 0 ? "100" : "0");
        tableHarmonics_->setItem(i, 1, valueItem);
    }

    harmLayout->addWidget(tableHarmonics_);
    mainLayout->addWidget(harmGroup);

    // ---- 计算按钮与结果 ----
    auto *btnLayout = new QHBoxLayout();
    auto *btnCalcTHD = new QPushButton("计算 THD", tab);
    btnCalcTHD->setMinimumHeight(36);
    btnLayout->addWidget(btnCalcTHD);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    connect(btnCalcTHD, &QPushButton::clicked, this, &MainWindow::onCalcTHD);

    auto *resultGroup = new QGroupBox("THD 计算结果", tab);
    auto *resultLayout = new QFormLayout(resultGroup);
    editTHDUResult_ = new QLineEdit(tab);
    editTHDUResult_->setReadOnly(true);
    labelTHDStatus_ = new QLabel(tab);
    labelTHDStatus_->setStyleSheet("font-weight: bold; font-size: 14px;");
    resultLayout->addRow("总谐波畸变率 THD：", editTHDUResult_);
    resultLayout->addRow("评估结果：", labelTHDStatus_);
    mainLayout->addWidget(resultGroup);

    mainLayout->addStretch();
}

void MainWindow::setupCostTab(QWidget *tab)
{
    auto *mainLayout = new QVBoxLayout(tab);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // ---- 输入参数 ----
    auto *inputGroup = new QGroupBox("用电参数", tab);
    auto *inputLayout = new QFormLayout(inputGroup);

    editLoadPowerKw_ = new QLineEdit(tab);
    editLoadPowerKw_->setPlaceholderText("如 5.5");
    editRatedPowerKw_ = new QLineEdit(tab);
    editRatedPowerKw_->setPlaceholderText("如 10");
    editHoursPerDay_ = new QLineEdit(tab);
    editHoursPerDay_->setPlaceholderText("默认24小时");
    editHoursPerDay_->setText("24");

    inputLayout->addRow("有功功率 (kW)：", editLoadPowerKw_);
    inputLayout->addRow("设备额定功率 (kW)：", editRatedPowerKw_);
    inputLayout->addRow("日运行小时数：", editHoursPerDay_);

    mainLayout->addWidget(inputGroup);

    // ---- 按钮 ----
    auto *btnLayout = new QHBoxLayout();
    auto *btnCalcCost = new QPushButton("计算电费 & 负载率", tab);
    btnCalcCost->setMinimumHeight(36);
    btnLayout->addWidget(btnCalcCost);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    connect(btnCalcCost, &QPushButton::clicked, this, &MainWindow::onCalcCost);

    // ---- 结果 ----
    auto *resultGroup = new QGroupBox("电费 & 负载率结果", tab);
    auto *resultLayout = new QVBoxLayout(resultGroup);
    textCostResult_ = new QTextEdit(tab);
    textCostResult_->setReadOnly(true);
    textCostResult_->setMinimumHeight(200);
    resultLayout->addWidget(textCostResult_);
    mainLayout->addWidget(resultGroup);
}

void MainWindow::onCalculate()
{
    bool okV, okI, okPF;
    double U = editVoltage_->text().toDouble(&okV);
    double I = editCurrent_->text().toDouble(&okI);
    double pf = editPowerFactor_->text().toDouble(&okPF);

    if (!okV || !okI || !okPF) {
        textResult_->setText("<b style='color:red;'>错误：请输入有效数值。</b>");
        return;
    }

    if (U <= 0 || I <= 0) {
        textResult_->setText("<b style='color:red;'>错误：电压和电流必须大于0。</b>");
        return;
    }

    if (pf < 0 || pf > 1) {
        textResult_->setText("<b style='color:red;'>错误：功率因数范围为 0 ~ 1。</b>");
        return;
    }

    int connIndex = comboConnection_->currentIndex();
    PowerCalculator::ConnectionType connType;
    QString connName;
    switch (connIndex) {
    case 0: connType = PowerCalculator::Star;   connName = "星形接法（Y）"; break;
    case 1: connType = PowerCalculator::Delta;  connName = "三角形接法（Δ）"; break;
    default: connType = PowerCalculator::SinglePhase; connName = "单相电路"; break;
    }

    PowerCalculator calc(connType);
    calc.setVoltage(U);
    calc.setCurrent(I);
    calc.setPowerFactor(pf);

    auto result = calc.calculate();

    QString html;
    html += "<h3>计算结果</h3>";
    html += QString("<table border='0' cellspacing='4'>");
    html += QString("<tr><td><b>接线方式：</b></td><td>%1</td></tr>").arg(connName);
    html += QString("<tr><td><b>线电压：</b></td><td>%1 V</td></tr>").arg(U, 0, 'f', 1);
    html += QString("<tr><td><b>线电流：</b></td><td>%1 A</td></tr>").arg(I, 0, 'f', 2);
    html += QString("<tr><td><b>功率因数 cosφ：</b></td><td>%1</td></tr>").arg(pf, 0, 'f', 3);
    html += "<tr><td colspan='2'><hr></td></tr>";
    html += QString("<tr><td><b>视在功率 S：</b></td><td>%1 VA</td></tr>").arg(result.apparentPower, 0, 'f', 2);
    html += QString("<tr><td><b>有功功率 P：</b></td><td>%1 W</td></tr>").arg(result.activePower, 0, 'f', 2);
    html += QString("<tr><td><b>无功功率 Q：</b></td><td>%1 var</td></tr>").arg(result.reactivePower, 0, 'f', 2);
    html += "</table>";

    html += "<br><b>功率因数评价：</b>";
    if (pf >= 0.95) {
        html += "<span style='color:green;'>优秀</span>";
    } else if (pf >= 0.85) {
        html += "<span style='color:darkgreen;'>良好</span>";
    } else if (pf >= 0.70) {
        html += "<span style='color:orange;'>一般，建议补偿</span>";
    } else {
        html += "<span style='color:red;'>偏低，需要无功补偿</span>";
    }

    textResult_->setHtml(html);
}

void MainWindow::onClear()
{
    editVoltage_->clear();
    editCurrent_->clear();
    editPowerFactor_->clear();
    textResult_->clear();
    comboConnection_->setCurrentIndex(0);
}

void MainWindow::onCalcTHD()
{
    // 从表格读取谐波数据
    std::vector<double> harmonics;
    for (int row = 0; row < tableHarmonics_->rowCount(); ++row) {
        auto *item = tableHarmonics_->item(row, 1);
        if (item) {
            bool ok;
            double val = item->text().toDouble(&ok);
            if (ok) {
                harmonics.push_back(val);
            }
        }
    }

    if (harmonics.empty() || harmonics[0] == 0.0) {
        editTHDUResult_->setText("错误：请输入谐波数据");
        labelTHDStatus_->setText("");
        return;
    }

    THDResult result = PowerCalculator::calculateTHD(harmonics);

    double voltageLevel = editVoltageTHD_->text().toDouble();
    result.passed = PowerCalculator::isTHDPass(result.thdU, result.thdI, voltageLevel);

    editTHDUResult_->setText(QString::number(result.thdU, 'f', 2) + " %");

    if (result.passed) {
        labelTHDStatus_->setText("✅ 合格 - 满足国标 GB/T 14549 限值");
        labelTHDStatus_->setStyleSheet("font-weight: bold; font-size: 14px; color: green;");
    } else {
        labelTHDStatus_->setText("❌ 不合格 - 超出谐波限值，需加装滤波器");
        labelTHDStatus_->setStyleSheet("font-weight: bold; font-size: 14px; color: red;");
    }
}

void MainWindow::onCalcCost()
{
    bool okP, okR, okH;
    double powerKw = editLoadPowerKw_->text().toDouble(&okP);
    double ratedKw = editRatedPowerKw_->text().toDouble(&okR);
    double hours = editHoursPerDay_->text().toDouble(&okH);

    if (!okP || !okR) {
        textCostResult_->setHtml("<b style='color:red;'>错误：请输入有效的功率数值。</b>");
        return;
    }

    if (!okH || hours <= 0) {
        hours = 24.0;
    }

    CostResult cost = PowerCalculator::calculateCost(powerKw, hours);
    QString loadEval = PowerCalculator::evaluateLoadRate(powerKw, ratedKw);

    QString html;
    html += "<h3>电费估算结果</h3>";
    html += QString("<table border='0' cellspacing='4'>");
    html += QString("<tr><td><b>设备有功功率：</b></td><td>%1 kW</td></tr>").arg(powerKw, 0, 'f', 2);
    html += QString("<tr><td><b>日运行小时数：</b></td><td>%1 h</td></tr>").arg(hours, 0, 'f', 1);
    html += QString("<tr><td><b>日用电量：</b></td><td>%1 kWh</td></tr>").arg(cost.dailyKWh, 0, 'f', 2);
    html += QString("<tr><td><b>月用电量（30天）：</b></td><td>%1 kWh</td></tr>").arg(cost.monthlyKWh, 0, 'f', 2);
    html += "<tr><td colspan='2'><hr></td></tr>";

    QString tierLabel;
    if (cost.tier == 1) tierLabel = "<span style='color:green;'>第一档（≤210 kWh）</span>";
    else if (cost.tier == 2) tierLabel = "<span style='color:orange;'>第二档（210~400 kWh）</span>";
    else tierLabel = "<span style='color:red;'>第三档（>400 kWh）</span>";

    html += QString("<tr><td><b>阶梯档位：</b></td><td>%1</td></tr>").arg(tierLabel);
    html += QString("<tr><td><b>日预估电费：</b></td><td>%1 元</td></tr>").arg(cost.dailyCost, 0, 'f', 2);
    html += QString("<tr><td><b>月预估电费：</b></td><td>%1 元</td></tr>").arg(cost.monthlyCost, 0, 'f', 2);
    html += "</table>";

    html += "<br><h3>负载率评估</h3>";
    html += QString("<p>%1</p>").arg(loadEval);

    textCostResult_->setHtml(html);
}

void MainWindow::onConnectionChanged(int index)
{
    Q_UNUSED(index);
    // 切换接线方式时自动更新提示
}
