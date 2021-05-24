/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "homeloansplitter.h"
#include "ui_homeloansplitter.h"

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLegend>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QApplication>
#include <QtCharts/QValueAxis>
#include <QLocale>

#include "loan.h"

HomeLoanSplitter::HomeLoanSplitter(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui_ThemeWidgetForm)
{
    m_ui->setupUi(this);
    this->setup();
    this->onCalculateClicked();
}

HomeLoanSplitter::~HomeLoanSplitter()
{
    delete m_ui;
}

void HomeLoanSplitter::onCalculateClicked()
{
    static QLocale locale = QLocale();
    m_ui->plainTextEditResult->clear();
    float totalLoan = m_ui->comboBoxTotalLoanAmount->itemData(m_ui->comboBoxTotalLoanAmount->currentIndex()).toFloat();
    float saving = m_ui->comboBoxSavingStart->itemData(m_ui->comboBoxSavingStart->currentIndex()).toFloat();
    float offsetStart = saving;
    float offsetStep = 1000;
    int totalLoanSteps = int((totalLoan - saving)/offsetStep) + 1;
    float savingIncremental = m_ui->comboBoxSavingIncremental->itemData(m_ui->comboBoxSavingIncremental->currentIndex()).toFloat();
    float fixRateYear = m_ui->comboBoxFixedInterestRate->itemData(m_ui->comboBoxFixedInterestRate->currentIndex()).toFloat() / 100.f;
    float varRateYear = m_ui->comboBoxVariableInterestRate->itemData(m_ui->comboBoxVariableInterestRate->currentIndex()).toFloat() / 100.f;
    int totalLoanYears = m_ui->comboBoxTotalLoanPeriod->itemData(m_ui->comboBoxTotalLoanPeriod->currentIndex()).toInt();
    float offsetAmount = m_ui->comboBoxOffsetAmount->itemData(m_ui->comboBoxOffsetAmount->currentIndex()).toFloat();
    int fixedYears = m_ui->comboBoxFixedTerm->itemData(m_ui->comboBoxFixedTerm->currentIndex()).toInt();
    int monthsPerYear = 12;
    int totalLoanFixedPeriod = fixedYears * monthsPerYear;

    float totalLoanInterestsDuringFixedTerm = .0f;
    float minInterests = totalLoan;
    float bestOffset = 0;
    int bestPaiedOffTerm = 0;
    float bestMonthlyRepaymentFixed = 0;
    float bestMaxMonthlyRepaymentVar = 0;

    if (totalLoan < offsetAmount)
    {
        m_ui->plainTextEditResult->appendHtml(QString("%1 Split into offset amount must less than your total loan %2").arg("<p style=\"color:red\">").arg("</p>"));
        m_chart->removeAllSeries();
        return;
    }

    if (offsetAmount < 0) // find me the best
    {
        for (int step = 0; step < totalLoanSteps; step ++)
        {
            m_ui->progressBarCalc->setValue(step * 100 / totalLoanSteps);
            qApp->processEvents();
            float offset = offsetStart + step * offsetStep;
            //fix part repayment and interesting
            float principalFix = std::max(totalLoan - offset, 0.f);
            auto loanFixedSchedule = Loan(principalFix, fixRateYear, totalLoanYears).schedule();
            float fixedPartTotalInterests = 0.f;
            for(int fixedTerm = 1; fixedTerm < totalLoanFixedPeriod + 1; fixedTerm ++)
            {
                fixedPartTotalInterests += loanFixedSchedule[fixedTerm].interest;
            }
            auto offsetSchedule = LoanWithOffset(offset, varRateYear, totalLoanYears, TermUnit::years, saving, savingIncremental);
            totalLoanInterestsDuringFixedTerm = fixedPartTotalInterests + offsetSchedule[totalLoanYears * monthsPerYear].totalInterest;
            if (totalLoanInterestsDuringFixedTerm < minInterests){
                minInterests = totalLoanInterestsDuringFixedTerm;
                bestOffset = offset;
            }
            // No need to continue the search if there is still a interest on offset account after the fixed term
            if (offsetSchedule[fixedYears * monthsPerYear + 1].interest > 0) {
                break;
            }
        }
    }
    else
    {
        bestOffset = std::max(offsetAmount, 0.f);
    }
    m_ui->progressBarCalc->setValue(100);

    m_chart->removeAllSeries();
    if (fixedYears > 0 && totalLoan - bestOffset > 1)
    {
        std::vector<Installment> fixedSchedule1, fixedSchedule2;
        if (totalLoan - bestOffset > 1)
        {
            // Fixed part schedule 1, should only use fixedYears items
            fixedSchedule1 = Loan(totalLoan - bestOffset, fixRateYear, totalLoanYears).schedule();
            bestMonthlyRepaymentFixed = fixedSchedule1[1].payment;
            // Fixed part schedule 2
            fixedSchedule2 = Loan(fixedSchedule1[fixedYears * monthsPerYear + 1].balance, varRateYear, totalLoanYears - fixedYears).schedule();
        }

        QLineSeries* seriesBalance = new QLineSeries();
        seriesBalance->setName("Loan Balance");
        auto offsetSchedule = LoanWithOffset(bestOffset, varRateYear, totalLoanYears, TermUnit::years, saving, savingIncremental);
        for(int index = 0; index < fixedYears * monthsPerYear + 1; index ++)
        {
            seriesBalance->append(index, fixedSchedule1[index].balance + offsetSchedule[index].balance);
        }
        for(int index = fixedYears * monthsPerYear + 1; index < totalLoanYears * monthsPerYear + 1; index++)
        {
            seriesBalance->append(index, fixedSchedule2[index - fixedYears * monthsPerYear].balance + offsetSchedule[index].balance);
        }
        auto it = std::find_if(++offsetSchedule.begin(), offsetSchedule.end(), [](const Installment& in) { return in.interest < 0.1;});
        bestPaiedOffTerm = it->number - 1;
        for(auto in : offsetSchedule)
        {
            bestMaxMonthlyRepaymentVar = std::max(bestMaxMonthlyRepaymentVar, in.payment);
        }

        m_chart->addSeries(seriesBalance);

        QScatterSeries *seriesFixEnd = new QScatterSeries();
        seriesFixEnd->setName("Fixed End");
        seriesFixEnd->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        seriesFixEnd->setMarkerSize(10.0);
        int index = fixedYears * monthsPerYear;
        seriesFixEnd->append(index, fixedSchedule1[index].balance + offsetSchedule[index].balance);
        m_chart->addSeries(seriesFixEnd);

        m_chart->createDefaultAxes();

        auto xList = m_chart->axes(Qt::Horizontal);
        xList[0]->setTitleText("Months");
        auto yList = m_chart->axes(Qt::Vertical);
        yList[0]->setTitleText("Balance");

        m_ui->plainTextEditResult->appendPlainText(QString("Monthly Repayment Fixed Part:  %1").arg(locale.toCurrencyString(bestMonthlyRepaymentFixed, "$ ")));
        m_ui->plainTextEditResult->appendPlainText(QString("Monthly Repayment Variable Part(max):  %1").arg(locale.toCurrencyString(bestMaxMonthlyRepaymentVar, "$ ")));
        m_ui->plainTextEditResult->appendHtml(QString("%1 Split Into Offset amount: %2 %3").arg("<p style=\"color:red\">").arg(locale.toCurrencyString(bestOffset, "$ ")).arg("</p>"));
        m_ui->plainTextEditResult->appendPlainText(QString("Split Part Interests Paied Off Term:  %1").arg(bestPaiedOffTerm));
        m_ui->plainTextEditResult->appendPlainText("Based On:");
        m_ui->plainTextEditResult->appendPlainText(QString("    Total Loan: %1 | Total Term: %2 year(s)").arg(locale.toCurrencyString(totalLoan, "$ ")).arg(totalLoanYears));
        m_ui->plainTextEditResult->appendPlainText(QString("    Fixed Rate: %1 % | Variable Rate: %2 %").arg(fixRateYear * 100.0).arg(varRateYear * 100.0));
        m_ui->plainTextEditResult->appendPlainText(QString("    Fixed Term: %1 year(s) | Fixed amount: %2").arg(fixedYears).arg(locale.toCurrencyString(totalLoan - bestOffset, "$ ")));
        m_ui->plainTextEditResult->appendPlainText(QString("    Offset Saving base: %1 | OffSet Saving Inremental: %2").arg(locale.toCurrencyString(saving, "$ ")).arg(locale.toCurrencyString(savingIncremental, "$ ")));

    }
    else // no best split, do normal repayment calc
    {
        auto schedule = Loan(totalLoan, varRateYear, totalLoanYears).schedule();
        QLineSeries* seriesBalance = new QLineSeries();
        seriesBalance->setName("Loan Balance");
        for(int index = 0; index < schedule.size(); index ++)
        {
            seriesBalance->append(schedule[index].number, schedule[index].balance);
        }
        m_chart->addSeries(seriesBalance);
        m_chart->createDefaultAxes();

        auto xList = m_chart->axes(Qt::Horizontal);
        xList[0]->setTitleText("Months");
        auto yList = m_chart->axes(Qt::Vertical);
        yList[0]->setTitleText("Balance");

        bestMonthlyRepaymentFixed = schedule[1].payment;
        m_ui->plainTextEditResult->appendPlainText("No Best Split option. Calculate the normal repayment.\n");
        m_ui->plainTextEditResult->appendPlainText(QString("Monthly Repayment:  %1").arg(locale.toCurrencyString(bestMonthlyRepaymentFixed, "$ ")));
        m_ui->plainTextEditResult->appendPlainText("Based On:");
        m_ui->plainTextEditResult->appendPlainText(QString("    Total Loan:  %1").arg(locale.toCurrencyString(totalLoan, "$ ")));
        m_ui->plainTextEditResult->appendPlainText(QString("    Interest Rate:  %1 %").arg(varRateYear * 100.0));
        m_ui->plainTextEditResult->appendPlainText(QString("    Total Term:  %1 year(s)").arg(totalLoanYears));
    }
}

void HomeLoanSplitter::setup()
{
    // setup total loan
    setupComboBox(m_ui->comboBoxTotalLoanAmount, 500000, 10000000, 10000, 850000, "$ ");
    m_ui->comboBoxTotalLoanAmount->addItem("Are u sure?", std::numeric_limits<float>().max()/10.f);

    // setup total loan period
    setupComboBox(m_ui->comboBoxTotalLoanPeriod, 1, 30, 1, 27, "", " year(s)");

    // setup var & fix interests
    setupComboBox(m_ui->comboBoxVariableInterestRate, 1.00, 5.00, 0.01, 2.65, "", " %");
    setupComboBox(m_ui->comboBoxFixedInterestRate, 1.00, 5.00, 0.01, 1.94, "", " %");

    // setup fixed amount/term
    setupComboBox(m_ui->comboBoxOffsetAmount, 0, 10000000, 10000, 0, "$ ");
    m_ui->comboBoxOffsetAmount->insertItem(0, "Find Me The Best", -1.f);
    m_ui->comboBoxOffsetAmount->setCurrentIndex(0);
    setupComboBox(m_ui->comboBoxFixedTerm, 0, 5, 1, 2, "", " year(s)");

    // setup saving start and incremental
    setupComboBox(m_ui->comboBoxSavingStart, 0, 1000000, 10000, 20000, "$ ");
    setupComboBox(m_ui->comboBoxSavingIncremental, 0, 100000, 1000, 6000, "$ ");

    connect(m_ui->comboBoxTotalLoanAmount, &QComboBox::currentIndexChanged, this, &HomeLoanSplitter::onCalculateClicked);
    connect(m_ui->comboBoxTotalLoanPeriod, &QComboBox::currentIndexChanged, this, &HomeLoanSplitter::onCalculateClicked);
    connect(m_ui->comboBoxVariableInterestRate, &QComboBox::currentIndexChanged, this, &HomeLoanSplitter::onCalculateClicked);
    connect(m_ui->comboBoxFixedInterestRate, &QComboBox::currentIndexChanged, this, &HomeLoanSplitter::onCalculateClicked);
    connect(m_ui->comboBoxOffsetAmount, &QComboBox::currentIndexChanged, this, &HomeLoanSplitter::onCalculateClicked);
    connect(m_ui->comboBoxFixedTerm, &QComboBox::currentIndexChanged, this, &HomeLoanSplitter::onCalculateClicked);
    connect(m_ui->comboBoxSavingStart, &QComboBox::currentIndexChanged, this, &HomeLoanSplitter::onCalculateClicked);
    connect(m_ui->comboBoxSavingIncremental, &QComboBox::currentIndexChanged, this, &HomeLoanSplitter::onCalculateClicked);

    m_chart = new QChart();
    m_chart->setTitle("Loan Balance Chart");
    //m_chart->legend()->hide();
    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_ui->verticalLayoutMostOuter->addWidget(m_chartView);
}

void HomeLoanSplitter::setupComboBox(QComboBox* comboBox, float startValue, float endValue, float step, float defultValue, QString prefix, QString surfix)
{
    static QLocale locale = QLocale();
    int index = static_cast<int>((defultValue - startValue) / step);
    for(float data = startValue; data <= endValue; data += step)
    {
        if (prefix.startsWith('$'))
        {
            comboBox->addItem(QString("%1%2").arg(locale.toCurrencyString(data, prefix)).arg(surfix), data);
        }
        else
        {
            comboBox->addItem(QString("%1%2%3").arg(prefix).arg(data).arg(surfix), data);
        }
    }
    comboBox->setCurrentIndex(index);
}

Schedule HomeLoanSplitter::LoanWithOffset(float principal, float interest, int term, TermUnit termUnit, float savingStart, float savingIncremental)
{
    Schedule schedule;
    schedule.push_back(Installment{.number = 0,
                                   .payment = .0f,
                                   .interest = .0f,
                                   .principal = .0f,
                                   .totalInterest = .0f,
                                   .balance = principal});
    int totalTermInMonths = term;
    switch (termUnit) {
    case TermUnit::days:
        totalTermInMonths = term / 30;
        break;
    case TermUnit::months:
        totalTermInMonths = term;
        break;
    case TermUnit::years:
        totalTermInMonths = term * 12;
        break;
    }
    if (principal < 0.1)
    {
        for(int i = 1; i < totalTermInMonths + 1; i++)
        {
            schedule.push_back(Installment{.number = i,
                                           .payment = .0f,
                                           .interest = .0f,
                                           .principal = .0f,
                                           .totalInterest = .0f,
                                           .balance = .0f});
        }

        return schedule;
    }
    float totalSavingInOffset = savingStart;
    float principalNoInterest = std::min(principal, totalSavingInOffset);
    float principalWithInterest = std::max(principal - totalSavingInOffset, 0.f);

    if (principalWithInterest < 0.1) {
        return Loan(principal, 0.f, term, termUnit).schedule();
    }

    for (int term = 0; term < totalTermInMonths; term ++) {
        Installment in;
        in.number = term + 1;
        in.totalInterest = schedule[term].totalInterest;
        in.balance = schedule[term].balance;

        // with interest part
        if (principalWithInterest > 1)
        {
            auto scheduleWithInterests = Loan(principalWithInterest, interest, totalTermInMonths - term, TermUnit::months).schedule();
            in.payment += scheduleWithInterests[1].payment;
            in.interest += scheduleWithInterests[1].interest;
            in.principal += scheduleWithInterests[1].principal;
            in.totalInterest += scheduleWithInterests[1].interest;
            in.balance -= scheduleWithInterests[1].principal;
        }

        // no interest part
        float monthlyRepaymentNoInterestPart = principalNoInterest / (totalTermInMonths - term);
        in.payment += monthlyRepaymentNoInterestPart;
        in.principal += monthlyRepaymentNoInterestPart;
        in.balance -= monthlyRepaymentNoInterestPart;
        schedule.push_back(in);

        totalSavingInOffset += savingIncremental;
        principalNoInterest = std::min(in.balance, totalSavingInOffset);
        principalWithInterest = std::max(in.balance - totalSavingInOffset, 0.f);
    }
    return schedule;
}
