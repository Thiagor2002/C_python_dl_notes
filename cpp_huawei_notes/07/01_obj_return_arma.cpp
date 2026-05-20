#include <armadillo>  // arma::imat
#include <iostream>   // std::cout

using arma::imat;
using std::cout;

int main()
{
    imat a{{1, 1}, {2, 2}};
    imat b{{1, 0}, {0, 1}};
    imat c{{2, 2}, {1, 1}};
    imat r = a * b + c;
    cout << r;
}
