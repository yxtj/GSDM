#ifndef __POWER_LAW_DISTRIBUTION_HPP
#define __POWER_LAW_DISTRIBUTION_HPP
#include <random>
#include <limits>
#include <stdexcept>

/*!
power_law_distribution
define domain: [Xmin, +inf).
f(X=x)=C*x^(-alpha);	C=(alpha-1)/Xmin^(1-alpha).
F(x)=cdf(x)=P(X<x)=1-(x/Xmin)^(1-alpha).
cF(y)=Xmin*(1-y)^(1/(1-alpha)).
*/

template <typename T = double>
class power_law_distribution
{
private:
	typedef std::uniform_real_distribution<double> urng_type;
public:
	typedef T	result_type;
	struct param_type {
		T xmin; double alpha;
		double _exp;	//short cut for random value generating.
		explicit param_type(T xmin, double alpha) { init(xmin, alpha); }
		void init(T xmin, double alpha) {
			if(xmin < 1)
				throw std::invalid_argument("invalid argument xmin (<1) for power_law_distribution");
			if(alpha <= 1)
				throw std::invalid_argument("invalid argument alpha (<=1) for power_law_distribution");
			this->xmin = xmin;
			this->alpha = alpha;
			_exp = 1.0 / (1 - alpha);
		}
		bool operator==(const param_type& p) const {	// test for equality
			return (xmin == p.xmin && alpha == p.alpha);
		}
		bool operator!=(const param_type& p) const {	// test for inequality
			return !(*this == p);
		}
	};

public:
	explicit power_law_distribution(T xmin = 1, double alpha = 2.5) :par(xmin, alpha) {}
	explicit power_law_distribution(const param_type& params) :par(params) {}

	result_type min()const { return xmin(); }
	result_type max()const { return std::numeric_limits<result_type>::max(); }
	param_type param()const { return par; }
	void param(const param_type& parm) { par = parm; }
	void reset() { urng.reset(); }	//comment if use dynamic urng
									//	void reset(){}

	result_type xmin()const { return par.xmin; }
	double alpha()const { return par.alpha; }

	template<class Engine>
	result_type operator()(Engine& g) { return eval(g, par); }
	template<class Engine>
	result_type operator()(Engine& g, const param_type& parm) { return eval(g, parm); }

	friend bool operator==(const power_law_distribution& lth,
		const power_law_distribution& rth) {
		return lth.par == rth.par;
	}
	friend bool operator!=(const power_law_distribution& lth,
		const power_law_distribution& rth) {
		return lth.par != rth.par;
	}

	template <class charT, class traits>
	std::basic_istream<charT, traits>& read(std::basic_istream<charT, traits>& is) {
		T xmin;
		double a;
		param_type p;
		is >> a >> p;
		par.init(xmin, a);
		par = p;
		return is;
	}
	template <class charT, class traits>
	std::basic_ostream<charT, traits>& write(std::basic_ostream<charT, traits>& os) const {
		return os << par.xmin << ' ' << par.alpha << ' ' << urng;
	}
private:
	template <class Engine>
	result_type eval(Engine& eng, const param_type& par) {
		//urng_type urng;	//uncomment if use dynamic urng
		return static_cast<result_type>(par.xmin*std::pow(1.0 - urng(eng), par._exp));
	}

private:
	urng_type urng;	//comment if use dynamic urng
	param_type par;
};

//template <class T>
//inline bool operator==(const power_law_distribution<T>& lth,
//	const power_law_distribution<T>& rth)
//{
//	return lth.par == rth.par;
//}

//template <class T>
//inline bool operator!=(const power_law_distribution<T>& lth,
//	const power_law_distribution<T>& rth)
//{
//	return lth.par != rth.par;
//}

template <class charT, class traits, class T>
inline std::basic_istream<charT, traits>& operator>>(std::basic_istream<charT, traits>& is,
	power_law_distribution<T>& distr)
{
	return distr.read(is);
}

template <class charT, class traits, class T>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& os,
	const power_law_distribution<T>& distr)
{
	return distr.write(os);
}

#endif