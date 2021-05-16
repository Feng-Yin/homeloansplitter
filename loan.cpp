#include "loan.h"

#include<cmath>

Loan::Loan(float principal, float interest, int term, TermUnit termUnit)
    :m_principal(principal),
      m_interest(interest),
      m_term(term)
{
    switch (termUnit) {
    case TermUnit::days:
        m_term_in_months = term / 30;
        break;
    case TermUnit::months:
        m_term_in_months = m_term;
        break;
    case TermUnit::years:
        m_term_in_months = m_term * 12;
        break;
    }

    m_interest_monthly = m_interest / 12.0f;
    m_compounded_peroids.emplace(std::make_pair<>(Compounded::annually, 1));
    m_compounded_peroids.emplace(std::make_pair<>(Compounded::monthly, 12));
    m_compounded_peroids.emplace(std::make_pair<>(Compounded::dayily, 365));

    calcMonthlyPayment();
    amortize();
}

const Schedule& Loan::schedule() const
{
    return m_schedule;
}

void Loan::amortize()
{
    m_schedule.push_back(Installment{.number = 0,
                                     .payment = .0f,
                                     .interest = .0f,
                                     .principal = .0f,
                                     .totalInterest = .0f,
                                     .balance = m_principal});
    float totalInterest = .0f;
    float balance = m_principal;
    for(int paymentNumber = 1; paymentNumber < m_term_in_months + 1; paymentNumber++){
        if (balance < 0.001)
        {
            m_schedule.push_back(Installment{.number = paymentNumber,
                                     .payment = .0f,
                                     .interest = .0f,
                                     .principal = .0f,
                                     .totalInterest = m_schedule[paymentNumber - 1].totalInterest,
                                     .balance = 0.f});
            continue;
        }
        float payment = monthlyPayment();
        auto payments = splitPayment(paymentNumber, payment);
        totalInterest += payments.first;
        if (payments.second <= balance)
        {
            balance -= payments.second;
        }
        else
        {
            payments.second = balance;
            payment = payments.first + payments.second;
            balance = 0;
        }
        m_schedule.push_back(Installment{.number = paymentNumber,
                                         .payment = payment,
                                         .interest = payments.first,
                                         .principal = payments.second,
                                         .totalInterest = totalInterest,
                                         .balance = balance});
    }
}

float Loan::calcMonthlyPayment()
{
    if (m_interest < 0.000001)
    {
        m_monthly_repayment = m_principal / m_term_in_months;
    }
    else
    {
        m_monthly_repayment = m_principal * m_interest_monthly / (1 - std::pow((1 + m_interest_monthly), - m_term_in_months));
    }
    return m_monthly_repayment;
}

float Loan::monthlyPayment() const
{
    return m_monthly_repayment;
}

std::pair<float, float> Loan::splitPayment(int number, float payment) const
{
    if (m_interest < 0.000001)
    {
        return {0.f, payment};
    }
    float interestPayment = computeInterestPortion(number);
    float principalPayment = payment - interestPayment;
    return {interestPayment, principalPayment};
}

float Loan::computeInterestPortion(int number) const
{
    float interestInMonthP1 = m_interest_monthly + 1.0;

    float numerator = m_principal * m_interest_monthly * (std::pow(interestInMonthP1, m_term_in_months + 1) - std::pow(interestInMonthP1, number));
    float denominator = interestInMonthP1 * (std::pow(interestInMonthP1, m_term_in_months) - 1);
    return numerator / denominator;
}

