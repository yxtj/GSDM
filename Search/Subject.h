#pragma once
#include "../common/Graph.h"
#include "../common/Motif.h"
#include <vector>
#include <functional>
class Subject
{
	int th;
	std::vector<Graph> gs;
	struct SDSignature;
	SDSignature* psign;
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

	void initSignature();

	void setTheta(const double theta);

	bool contain(const Edge& e) const;
	bool contain(const Motif& m) const;

private:
	std::function<bool(const Edge&)> fCE;
	std::function<bool(const Motif&)> fCM;

	bool _contain_e_normal(const Edge& e) const;
	bool _contain_m_normal(const Motif& m) const;

	bool _contain_e_sd(const Edge& e) const;
	bool _contain_m_sd(const Motif& m) const;
};

