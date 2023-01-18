#include <librapid/librapid.hpp>

#if defined(LIBRAPID_USE_MULTIPREC)

namespace librapid {
	std::string str(const mpz &val, int64_t, int base) { return val.get_str(base); }

	std::string str(const mpf_class &val, int64_t, int base) {
		mp_exp_t exp;
		auto res = val.get_str(exp, base);

		if (exp > 0) {
			if (static_cast<size_t>(exp) >= res.length())
				res += std::string(static_cast<size_t>(exp) - res.length() + 1, '0');
			res.insert(exp, ".");
			return res;
		} else {
			std::string tmp(-exp + 1, '0');
			tmp += res;
			tmp.insert(1, ".");
			return tmp;
		}
	}

	std::string str(const mpq &val, int64_t, int base) { return val.get_str(base); }

	std::string str(const mpfr &val, int64_t digits, int) {
		std::stringstream ss;
		ss << std::fixed;
		mp_prec_t dig2 = val.getPrecision();
		dig2		   = ::mpfr::bits2digits(digits < 0 ? dig2 : static_cast<mp_prec_t>(digits));
		ss.precision(dig2);
		ss << val;
		return ss.str();
	}
} // namespace librapid

#endif // LIBRAPID_USE_MULTIPREC