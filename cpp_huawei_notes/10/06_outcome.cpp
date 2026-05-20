#include <climits>            // INT_MIN
#include <ios>                // std::boolalpha
#include <iostream>           // std::cout/endl
#include <string>             // std::string
#include <system_error>       // std::errc
#include <boost/outcome.hpp>  // boost::outcome_v2

using namespace std;
namespace outcome = boost::outcome_v2;

enum class div_errc {
    success,
    divide_by_zero,
    integer_divide_overflows,
    not_integer_division,
};

namespace std {

template <>
struct is_error_code_enum<div_errc> : true_type {
};

}  // namespace std

class div_errc_category : public std::error_category {
public:
    const char* name() const noexcept override { return "divide error"; }
    std::string message(int c) const override
    {
        switch (c) {
        case static_cast<int>(div_errc::success):
            return "Successful";
        case static_cast<int>(div_errc::divide_by_zero):
            return "Divide by zero";
        case static_cast<int>(div_errc::integer_divide_overflows):
            return "Integer divide overflows";
        case static_cast<int>(div_errc::not_integer_division):
            return "Not integer division";
        }
        return "Unknown";
    }
    // OPTIONAL: Allow generic error conditions to be compared
    std::error_condition default_error_condition(int c) const
        noexcept override
    {
        switch (c) {
        case static_cast<int>(div_errc::success):
            break;
        case static_cast<int>(div_errc::divide_by_zero):
            return make_error_condition(std::errc::invalid_argument);
        case static_cast<int>(div_errc::integer_divide_overflows):
            return make_error_condition(std::errc::value_too_large);
        case static_cast<int>(div_errc::not_integer_division):
            return make_error_condition(std::errc::result_out_of_range);
        }
        return std::error_condition(c, *this);
    }
};

std::error_code make_error_code(div_errc e)
{
    static div_errc_category category;
    return {static_cast<int>(e), category};
}

outcome::result<int, std::error_code> safe_divide(int i, int j)
{
    if (j == 0) {
        return outcome::failure(div_errc::divide_by_zero);
    }
    if (i == INT_MIN && j == -1) {
        return outcome::failure(div_errc::integer_divide_overflows);
    }
    if (i % j != 0) {
        return outcome::failure(div_errc::not_integer_division);
    }
    return outcome::success(i / j);
}

outcome::result<int, std::error_code> safe_add_divide(int i, int j, int k)
{
    //auto&& result = safe_divide(j, k);
    //if (!result) {
    //    return result.as_failure();
    //}
    //auto&& q = result.value();
    //return i + q;
    BOOST_OUTCOME_TRY(q, safe_divide(j, k));
    return i + q;
    //return i + BOOST_OUTCOME_TRYX(safe_divide(j, k));
}

void print_error(ostream& os, const std::error_code& ec,
                 const char* end = "")
{
    os << ec.category().name() << ": " << ec.message() << end;
}

void print_error(ostream& os, const std::error_condition& ec,
                 const char* end = "")
{
    os << ec.category().name() << ": " << ec.message() << end;
}

ostream& operator<<(ostream& os, const outcome::result<int, std::error_code>& exp)
{
    if (exp) {
        os << exp.value();
    } else {
        print_error(os, exp.error());
    }
    return os;
}

void check(const outcome::result<int, std::error_code>& result)
{
    cout << result;
    if (!result && result.error() == div_errc::divide_by_zero) {
        cout << ": Are you serious?";
    } else if (result && result.value() == 42) {
        cout << ": Ha, I got you!";
    }
    cout << endl;
}

int main()
{
    cout << "*** Error code test\n";
    print_error(cout, std::errc::address_not_available, "\n");
    print_error(cout, div_errc(10), "\n");
    print_error(cout, div_errc::divide_by_zero, "\n");
    cout << std::boolalpha
         << (make_error_code(div_errc::divide_by_zero) ==
             std::errc::invalid_argument)
         << endl;
    cout << "*** Outcome test\n";
    check(safe_add_divide(2, 1, 0));
    check(safe_add_divide(37, 20, 7));
    check(safe_add_divide(39, 21, 7));
}
