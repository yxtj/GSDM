#pragma once
#include "Subject.h"
#include <vector>

class DataHolder
{
	std::vector<Subject> ss;
	mutable unsigned long long nSubjectChecked;
	mutable unsigned long long nEdgeChecked;
	mutable unsigned long long nMotifChecked;
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
	bool contain(const MotifBuilder& m, const double minPortion) const;
	int count(const Edge& e) const;
	int count(const MotifBuilder& m) const;

	unsigned long long getnSubjectChecked() const;
	unsigned long long getnEdgeChecked() const;
	unsigned long long getnMotifChecked() const;

	void initSignature();
	void initSignature(const int from, const int to);

	SDSignature* getSignature(const int idx);
	const SDSignature* getSignature(const int idx) const;
	void setSignature(const int idx, const SDSignature& sign);
	void setSignature(const int idx, SDSignature&& sign);

private:
	using pfConE_t = bool(DataHolder::*)(const Edge &, const double)const;
	using pfConM_t = bool(DataHolder::*)(const MotifBuilder &, const double)const;
	using pfCountE_t = int(DataHolder::*)(const Edge &)const;
	using pfCountM_t = int(DataHolder::*)(const MotifBuilder &)const;
	pfConE_t fConE;
	pfConM_t fConM;
	pfCountE_t fCountE;
	pfCountM_t fCountM;

	void bindCheckFunNormal();
	void bindCheckFunSD();

	bool _contain_e_normal(const Edge& e, const double minPortion) const;
	bool _contain_m_normal(const MotifBuilder& m, const double minPortion) const;
	int _count_e_normal(const Edge& e) const;
	int _count_m_normal(const MotifBuilder& m) const;

	bool _contain_e_sd(const Edge& e, const double minPortion) const;
	bool _contain_m_sd(const MotifBuilder& m, const double minPortion) const;
	int _count_e_sd(const Edge& e) const;
	int _count_m_sd(const MotifBuilder& m) const;

};

