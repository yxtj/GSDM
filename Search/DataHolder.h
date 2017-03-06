#pragma once
#include "Subject.h"
#include <vector>

class DataHolder
{
	std::vector<Subject> ss;

public:
	DataHolder();
	~DataHolder();

	void addSubject(const Subject& s);
	void addSubject(Subject&& s);
	
	size_t size() const;
	bool empty() const;
	int getnNode() const;

	std::vector<Subject>& get();
	const std::vector<Subject>& get() const;
	Subject& get(const int idx);
	const Subject& get(const int idx) const;

	void setTheta(const double theta);
	bool contain(const Edge& e, const double minPortion) const;
	bool contain(const Motif& m, const double minPortion) const;
	int count(const Edge& e) const;
	int count(const Motif& m) const;

	void initSignature();
	void initSignature(const int from, const int to);

	SDSignature* getSignature(const int idx);
	const SDSignature* getSignature(const int idx) const;
	void setSignature(const int idx, const SDSignature& sign);
	void setSignature(const int idx, SDSignature&& sign);

private:
	std::function<bool(const Edge&, const double)> fConE;
	std::function<bool(const Motif&, const double)> fConM;
	std::function<int(const Edge&)> fCountE;
	std::function<int(const Motif&)> fCountM;

	void bindCheckFunNormal();
	void bindCheckFunSD();

	bool _contain_e_normal(const Edge& e, const double minPortion) const;
	bool _contain_m_normal(const Motif& m, const double minPortion) const;
	int _count_e_normal(const Edge& e) const;
	int _count_m_normal(const Motif& m) const;

	bool _contain_e_sd(const Edge& e, const double minPortion) const;
	bool _contain_m_sd(const Motif& m, const double minPortion) const;
	int _count_e_sd(const Edge& e) const;
	int _count_m_sd(const Motif& m) const;

};

