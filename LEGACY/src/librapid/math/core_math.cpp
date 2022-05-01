#include <chrono>
#include <librapid/autocast/custom_complex.hpp>
#include <librapid/math/constants.hpp>
#include <librapid/math/core_math.hpp>
#include <random>
#include <string>
#include <vector>

namespace librapid {
	int64_t product(const std::vector<int64_t> &vals) {
		int64_t res = 1;
		for (const auto &val : vals) res *= val;
		return res;
	}

	int64_t product(const int64_t *vals, int64_t num) {
		int64_t res = 1;
		for (int64_t i = 0; i < num; i++) res *= vals[i];
		return res;
	}

	double product(const std::vector<double> &vals) {
		double res = 1;
		for (const auto &val : vals) res *= val;
		return res;
	}

	double product(const double *vals, int64_t num) {
		double res = 1;
		for (int64_t i = 0; i < num; i++) res *= vals[i];
		return res;
	}

	bool anyBelow(const std::vector<int64_t> &vals, int64_t bound) {
		for (const auto &val : vals)
			if (val < bound) return true;
		return false;
	}

	bool anyBelow(const int64_t *vals, int64_t dims, int64_t bound) {
		for (int64_t i = 0; i < dims; i++)
			if (vals[i] < bound) return true;
		return false;
	}

	double map(double val, double start1, double stop1, double start2,
			   double stop2) {
		return start2 + (stop2 - start2) * ((val - start1) / (stop1 - start1));
	}

	double pow10(int64_t exponent) {
		const static double pows[] = {
		  0.00001, 0.0001, 0.001, 0.01, 0.1, 1, 10, 100, 1000, 10000, 100000};
		if (exponent >= -5 && exponent <= 5) return pows[exponent + 5];

		double res = 1;

		if (exponent > 0)
			for (int64_t i = 0; i < exponent; i++) res *= 10;
		else
			for (int64_t i = 0; i > exponent; i--) res *= 0.1;

		return res;
	}

	double round(double num, int64_t dp) {
		const double alpha = pow10(dp);
		const double beta  = pow10(-dp);

		const double absx = abs(num * alpha);
		double y		  = floor(absx);

		if (absx - y >= 0.5) y += 1;

		return (num >= 0 ? y : -y) * beta;
	}

	double roundSigFig(double num, int64_t figs) {
		if (figs <= 0)
			throw std::invalid_argument(
			  "Cannot round to " + std::to_string(figs) +
			  " significant figures. Must be greater than 0");

		double tmp = num > 0 ? num : -num;
		int64_t n  = 0;

		while (tmp > 10) {
			tmp /= 10;
			++n;
		}

		while (tmp < 1) {
			tmp *= 10;
			--n;
		}

		return (tmp > 0 ? 1 : -1) * (round(tmp, figs - 1) * pow10(n));
	}

	uint64_t nthFibonacci(uint8_t n) {
		if (n >= 100) {
			throw std::overflow_error(
			  "Numbers above 100 are not allowed as argument.");
		}

		if (n <= 75) // As the C++ standard only defines long double to be at
					 // least 2x the precision of a regular double, we check if
					 // that limit is exceeded or not.
		{
			return (uint64_t) roundl(std::pow(0.5 * (1.0 + SQRT5), (long double) n)) / SQRT5;
		}

		int a = 0, b = 1, c, i;
		for (i = 2; i <= n; i++) {
			c = a + b;
			a = b;
			b = c;
		}
		return b;
	}

	void betterFcknBeEven(int64_t n) {
		if (!(n & 1)) return;
		fmt::print("[ ERROR ] Number was not even\n");
		std::exit(69);
	}
} // namespace librapid