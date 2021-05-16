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

#ifndef THEMEWIDGET_H
#define THEMEWIDGET_H

#include <QtWidgets/QWidget>
#include <QtCharts/QChartGlobal>

#include "loan.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class Ui_ThemeWidgetForm;
class QChartView;
class QChart;
QT_END_NAMESPACE


QT_USE_NAMESPACE

class HomeLoanSplitter: public QWidget
{
    Q_OBJECT
public:
    explicit HomeLoanSplitter(QWidget *parent = 0);
    ~HomeLoanSplitter();

private Q_SLOTS:
    void onCalculateClicked();

private:
    void setup();
    void setupComboBox(QComboBox* comboBox, float startValue, float endValue, float step, float defultValue, QString prefix="", QString surfix="");
    Schedule LoanWithOffset(float principal, float interest, int term, TermUnit termUnit = TermUnit::years, float savingStart = 0.f, float savingIncremental = 0.f);
private:
    Ui_ThemeWidgetForm *m_ui;
    QChart* m_chart;
    QChartView* m_chartView;

};

#endif /* THEMEWIDGET_H */
