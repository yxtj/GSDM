#pragma once
#include "../common/Graph.h"
#include "../common/MotifBuilder.h"
#include <vector>
#include <functional>

struct SDSignature;

class Subject
{
	int th;
	std::vector<Graph> gs;
	SDSignature* psign;
	
	static unsigned long long nGraphChecked;
public:
	Subject();
	~Subject();

	void addGraph(const Graph& g);
	void addGraph(Graph&& g);
	void addGraph(const std::vector<Graph>& gs);
	void addGraph(std::vector<Graph>&& gs);
	void setGraph(const std::vector<Graph>& gs);
	void setGraph(std::vector<Graph>&& gs);

	size_t size() const;
	bool empty() const;
	int getnNode() const;

	std::vector<Graph>& get();
	const std::vector<Graph>& get() const;
	Graph& get(const int idx);
	const Graph& get(const int idx) const;

	void setTheta(const double theta);

	std::vector<bool> enumerate(const Motif& m) const;

	bool contain_normal(const Edge& e, int req) const;
	bool contain_normal(const MotifBuilder& m, int req) const;
	bool containByPeriod_normal(const Edge& e, int step) const;
	bool containByPeriod_normal(const MotifBuilder& m, int step) const;

	bool contain_normal(const Edge& e) const;
	bool contain_normal(const MotifBuilder& m) const;
	bool containByPeriod_normal(const Edge& e) const;
	bool containByPeriod_normal(const MotifBuilder& m) const;

	bool contain_sd(const Edge& e) const;
	bool contain_sd(const MotifBuilder& m, const SDSignature& ms) const;
	bool containByPeriod_sd(const Edge& e) const;
	bool containByPeriod_sd(const MotifBuilder& m, const SDSignature& ms) const;

	static unsigned long long getnGraphChecked();

	void initSignature();
	bool checkSDNecessary(const MotifBuilder& m, const SDSignature & ms) const;

	SDSignature* getSignature();
	const SDSignature* getSignature() const;
	void setSignature(const SDSignature& sign);
	void setSignature(SDSignature&& sign);

	/* Helpers */
private:
	int nNodeQuick() const;

	/* Signature implemenation */
private:
	SDSignature* generateSignature();

};

