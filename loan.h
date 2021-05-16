#ifndef LOAN_H
#define LOAN_H

#include <map>
#include <vector>


// Unit for the lifespan of the loan.
enum class TermUnit : char {
    days,
    months,
    years
};

// Frequency that interest is compounded
enum class Compounded : char {
    dayily,
    monthly,
    annually
};

struct Installment {
    int number = 0;
    float payment = 0.f;
    float interest = 0.f;
    float principal = 0.f;
    float totalInterest = .0f;
    float balance = .0f;
};

using Schedule = std::vector<Installment>;

class Loan
{
public:
    Loan(float principal, float interest, int term, TermUnit termUnit = TermUnit::years);
    const Schedule& schedule() const;
    float monthlyPayment() const;

private:
    void amortize();
    float calcMonthlyPayment();
    std::pair<float, float> splitPayment(int number, float payment) const;
    float computeInterestPortion(int number) const;

private:
    float m_principal = .0f;
    float m_interest = .0f;
    int m_term = 0;
    int m_term_in_months = 0;
    float m_interest_monthly = .0f;

    std::map<Compounded, int> m_compounded_peroids;
    std::vector<Installment> m_schedule;

    float m_monthly_repayment = -1.0f;
};

#endif // LOAN_H
