#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QTabWidget>
#include <QTableWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    void onCalculate();
    void onClear();
    void onCalcTHD();
    void onConnectionChanged(int index);

private:
    void setupUI();
    void setupPowerTab(QWidget *tab);
    void setupTHDTab(QWidget *tab);

    // ---- 功率计算页 ----
    QComboBox *comboConnection_;
    QLineEdit *editVoltage_;
    QLineEdit *editCurrent_;
    QLineEdit *editPowerFactor_;
    QTextEdit *textResult_;

    // ---- 谐波分析页 ----
    QLineEdit *editVoltageTHD_;
    QTableWidget *tableHarmonics_;
    QLineEdit *editTHDUResult_;
    QLabel *labelTHDStatus_;

    QTabWidget *tabWidget_;
};

#endif // MAINWINDOW_H
