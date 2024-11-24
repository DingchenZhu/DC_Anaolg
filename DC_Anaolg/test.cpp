#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Core>

using namespace std;

int main()
{
	Eigen::Matrix<float, 2, 3> matrix_23;
	matrix_23 << 1, 2, 3, 4, 5, 6;
	cout << matrix_23 << endl;
	return 0;
}
