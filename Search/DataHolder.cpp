#include "stdafx.h"
#include "DataHolder.h"
#include "SDSignature.h"

using namespace std;

DataHolder::DataHolder()
{
	bindCheckFunNormal();
}


DataHolder::~DataHolder()
{
}

void DataHolder::addSubject(const Subject & s)
{
	ss.push_back(s);
}

void DataHolder::addSubject(Subject && s)
{
	ss.push_back(move(s));
}

size_t DataHolder::size() const
{
	return ss.size();
}

bool DataHolder::empty() const
{
	return ss.empty();
}

int DataHolder::getnNode() const
{
	return ss.empty() ? 0 : ss.front().getnNode();
}

std::vector<Subject>& DataHolder::get()
{
	return ss;
}

const std::vector<Subject>& DataHolder::get() const
{
	return ss;
}

Subject & DataHolder::get(const int idx)
{
	return ss[idx];
}

const Subject & DataHolder::get(const int idx) const
{
	return ss[idx];
}

void DataHolder::setTheta(const double theta)
{
	for(auto& s : ss)
		s.setTheta(theta);
}

bool DataHolder::contain(const Edge & e, const double minPortion) const
{
	return fConE(e, minPortion);
}

bool DataHolder::contain(const Motif & m, const double minPortion) const
{
	return fConM(m, minPortion);
}

int DataHolder::count(const Edge & e) const
{
	return fCountE(e);
}

int DataHolder::count(const Motif & m) const
{
	return fCountM(m);
}

void DataHolder::initSignature()
{
	for(auto& s : ss)
		s.initSignature();
	bindCheckFunSD();
}

void DataHolder::initSignature(const int from, const int to)
{
	for(int i = from; i < to; ++i)
		ss[i].initSignature();
	bindCheckFunSD();
}

SDSignature * DataHolder::getSignature(const int idx)
{
	return ss[idx].getSignature();
}

const SDSignature * DataHolder::getSignature(const int idx) const
{
	return ss[idx].getSignature();
}

void DataHolder::setSignature(const int idx, const SDSignature & sign)
{
	ss[idx].setSignature(sign);
}

void DataHolder::setSignature(const int idx, SDSignature && sign)
{
	ss[idx].setSignature(move(sign));
}

// ----------- Check Functions -----------

void DataHolder::bindCheckFunNormal()
{
	fConE = bind(&DataHolder::_contain_e_normal, this, placeholders::_1, placeholders::_2);
	fConM = bind(&DataHolder::_contain_m_normal, this, placeholders::_1, placeholders::_2);

	fCountE = bind(&DataHolder::_count_e_normal, this, placeholders::_1);
	fCountM = bind(&DataHolder::_count_m_normal, this, placeholders::_1);
}

void DataHolder::bindCheckFunSD()
{
	fConE = bind(&DataHolder::_contain_e_sd, this, placeholders::_1, placeholders::_2);
	fConM = bind(&DataHolder::_contain_m_sd, this, placeholders::_1, placeholders::_2);

	fCountE = bind(&DataHolder::_count_e_sd, this, placeholders::_1);
	fCountM = bind(&DataHolder::_count_m_sd, this, placeholders::_1);
}

bool DataHolder::_contain_e_normal(const Edge & e, const double minPortion) const
{
	int th = static_cast<int>(ceil(ss.size()*minPortion));
	// return true if #occurence >= th
	for(auto& sub : ss) {
		//++stNumSubjectChecked;
		if(sub.contain_normal(e)) {
			if(--th <= 0)
				return true;
		}
	}
	return false;
}

bool DataHolder::_contain_m_normal(const Motif & m, const double minPortion) const
{
	int th = static_cast<int>(ceil(ss.size()*minPortion));
	// return true if #occurence >= th
	for(auto& sub : ss) {
		//++stNumSubjectChecked;
		if(sub.contain_normal(m)) {
			if(--th <= 0)
				return true;
		}
	}
	return false;
}

int DataHolder::_count_e_normal(const Edge & e) const
{
	int cnt = 0;
	for(auto& sub : ss) {
		//++stNumSubjectChecked;
		if(sub.contain_normal(e)) {
			++cnt;
		}
	}
	return cnt;
}

int DataHolder::_count_m_normal(const Motif & m) const
{
	int cnt = 0;
	for(auto& sub : ss) {
		//++stNumSubjectChecked;
		if(sub.contain_normal(m)) {
			++cnt;
		}
	}
	return cnt;
}

bool DataHolder::_contain_e_sd(const Edge & e, const double minPortion) const
{
	int th = static_cast<int>(ceil(ss.size()*minPortion));
	for(auto& sub : ss) {
		//++stNumSubjectChecked;
		if(sub.contain_sd(e)) {
			if(--th <= 0)
				return true;
		}
	}
	return false;
}

bool DataHolder::_contain_m_sd(const Motif & m, const double minPortion) const
{
	SDSignature ms(m, getnNode());
	int th = static_cast<int>(ceil(ss.size()*minPortion));
	// return true if #occurence >= th
	for(auto& sub : ss) {
		//++stNumSubjectChecked;
		if(sub.contain_sd(m, ms)) {
			if(--th <= 0)
				return true;
		}
	}
	return false;
}

int DataHolder::_count_e_sd(const Edge & e) const
{
	int cnt = 0;
	for(auto& sub : ss) {
		//++stNumSubjectChecked;
		if(sub.contain_sd(e)) {
			++cnt;
		}
	}
	return cnt;
}

int DataHolder::_count_m_sd(const Motif & m) const
{
	SDSignature ms(m, getnNode());
	int cnt = 0;
	for(auto& sub : ss) {
		//++stNumSubjectChecked;
		if(sub.contain_sd(m, ms)) {
			++cnt;
		}
	}
	return cnt;
}
