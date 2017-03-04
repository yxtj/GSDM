#include "stdafx.h"
#include "DataHolder.h"

using namespace std;

DataHolder::DataHolder()
{
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

void DataHolder::setTheta(const double theta)
{
	for(auto& s : ss)
		s.setTheta(theta);
}

bool DataHolder::contain(const Edge & e, const double minPortion) const
{
	int th = static_cast<int>(ceil(ss.size()*minPortion));
	// return true if #occurence >= th
	for(auto& sub : ss) {
		//++stNumSubjectChecked;
		if(sub.contain(e)) {
			if(--th <= 0)
				return true;
		}
	}
	return false;
}

bool DataHolder::contain(const Motif & m, const double minPortion) const
{
	int th = static_cast<int>(ceil(ss.size()*minPortion));
	// return true if #occurence >= th
	for(auto& sub : ss) {
		//++stNumSubjectChecked;
		if(sub.contain(m)) {
			if(--th <= 0)
				return true;
		}
	}
	return false;
}

int DataHolder::count(const Edge & e) const
{
	int cnt = 0;
	for(auto& sub : ss) {
		//++stNumSubjectChecked;
		if(sub.contain(e)) {
			++cnt;
		}
	}
	return cnt;
}

int DataHolder::count(const Motif & m) const
{
	int cnt = 0;
	for(auto& sub : ss) {
		//++stNumSubjectChecked;
		if(sub.contain(m)) {
			++cnt;
		}
	}
	return cnt;
}
