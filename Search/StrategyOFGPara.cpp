#include "stdafx.h"
#include "StrategyOFGPara.h"
#include "Option.h"
#include "../util/Timer.h"
#include"MType.h"
#include <regex>

using namespace std;

const std::string StrategyOFGPara::name("ofg-para");
const std::string StrategyOFGPara::usageDesc(
	"Select the common frequent motifs as result.\n"
	"Usage: " + StrategyOFGPara::name + " <k> <theta> <obj-fun> <alpha> [sd] [net] [dces] [log]"
);
const std::string StrategyOFGPara::usageParam(
	"  <k>: [integer] return top-k result\n"
	//	"  <minSup>: [double] the minimum show up probability of a motif among positive subjects\n"
	"  <theta>: [double] the minimum show up probability of a motif among the snapshots of a subject\n"
	"  <obj-fun>: [string] name for the objective function (supprot: diff, margin, ratio)\n"
	"  <alpha>: [double] the penalty factor for the negative frequency\n"
	"  <dist>: optional [dist/dist-no], default disabled, run in distributed manner\n"
	"  [sd]: optional [sd/sd-no], default enabled, use the shortest distance optimization\n"
	"  [net]: optional [net/net-no], default enabled, use the motif network to prune (a motif's all parents should be valid)\n"
	"  [dces]: optional [dces/dces-c/dces-b/decs-no](:<ms>), "
	"default enabled, use dynamic candidate edge set ('c' for connected-condition, 'b' for bound-condition) "
	"<ms> is the minimum frequency of candidate edges, default 0.0\n"
	"  [npar]: opetional [npar-estim/npar-exact], default estimated, which way used to calculated the number of parents of a motif\n"
	"  [log]: optional [log:<path>/log-no], default disabled, output the score of the top-k result to given path"
);
const std::string StrategyOFGPara::usage = StrategyOFGPara::usageDesc + "\n" + StrategyOFGPara::usageParam;

bool StrategyOFGPara::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 4, 8, name);
		k = stoi(param[1]);
		pSnap = stod(param[2]);
		if(!setObjFun(param[3]))
			throw invalid_argument("Unsupported objective function for Strategy " + name + " : " + param[3]);
		// TODO: change to use a separated functio to parse the parameters for certain objective function
		if(objFunID == 1)
			alpha = stod(param[4]);
		flagUseSD = true;
		flagNetworkPrune = true;
		flagDCESConnected = true;
		flagDCESBound = true;
		minSup = 0.0;
		flagOutputScore = false;
		regex reg_sd("sd(-no)?");
		regex reg_net("net(-no)?");
		regex reg_dces("dces(-[cb])?(-no)?(:0\\.\\d+)?");
		regex reg_log("log(:.+)?(-no)?");
		for(size_t i = 5; i < param.size(); ++i) {
			smatch m;
			if(regex_match(param[i], m, reg_sd)) {
				bool flag = !m[1].matched;
				flagUseSD = flag;
			} else if(regex_match(param[i], m, reg_net)) {
				bool flag = !m[1].matched;
				flagNetworkPrune = flag;
			} else if(regex_match(param[i], m, reg_dces)) {
				bool flag = !m[2].matched;
				parseDCES(m[1], m[3], flag);
			} else if(regex_match(param[i], m, reg_log)) {
				bool flag = !m[2].matched;
				parseLOG(m[1], flag);
			} else {
				throw invalid_argument("Unknown option for strategy " + name + ": " + param[i]);
			}
		}
		// TODO: consider whether to put in an option
		setDCESmaintainOrder(false);
	} catch(exception& e) {
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

std::vector<Motif> StrategyOFGPara::search(const Option & opt, const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	if(!checkInput(gPos, gNeg))
		return std::vector<Motif>();
	// step 0: initialization: parameters, message driver, network thread, message thread
	net = NetworkThread::GetInstance();
	initParams(gPos, gNeg); //should be put after the initialization of net
	initStatistics();
	initHandlers(); // should be put after the initialization fo net
	running_ = true;
	thread tRecvMsg(bind(&StrategyOFGPara::messageReceiver, this));

	// step 1: register all workers
	registerAllWorkers();
	if(net->id() == 0)
		cout << "[LOG] all workers started." << endl;

	// step 2: initial candidate edge set
	Timer t;
	initialCE_para();
	if(net->id() == 0)
		cout << "[LOG] candidate edges initialized: " << edges.size() << " in all." << endl;

	// step 3: search independently (each worker holds motif whose score is larger than current k-th)
	work_para();
	{
		ostringstream oss;
		oss << "[LOG : " << net->id() << "] finishes searching.\n";
		cout << oss.str();
		cout.flush();
	}

	// step 4: coordinate the top-k motifs of all workers
	cooridnateTopK();
	if(net->id() == 0) {
		cout << "[LOG] global top-k motifs gathered." << endl;
	}
	vector<Motif> res = holder->getResultMove();
	delete holder;

	// step 5: merge statistics to Rank 0
	gatherStatistics();
	auto ts = t.elapseS();
	if(net->id() == 0) {
		cout << "[LOG] statistical information gathered." << endl;
		cout << "  Finished in " << ts << " seconds\n"
			<< "    motif explored " << stNumMotifExplored << " , generated " << stNumMotifGenerated << "\n"
			<< "    subject counted: " << stNumSubjectChecked << " , graph counted: " << stNumGraphChecked
			<< " , on average: " << (double)stNumGraphChecked / stNumSubjectChecked << " graph/subject\n"
			<< "    frequency calculated on positive: " << stNumFreqPos << " , on negative: " << stNumFreqNeg << endl;

	}

	running_ = false;
	tRecvMsg.join();

	return res;
}

void StrategyOFGPara::initParams(
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
{
	StrategyOFG::initParams(gPos, gNeg);
	running_ = true;
	lowerBound = numeric_limits<decltype(lowerBound)>::lowest();
	holder = new TopKBoundedHolder<Motif, double>(k);
	searchFinished = false;
}

void StrategyOFGPara::initLRTables()
{
	double lb = edges.back().second;
	ltable.init(&StrategyOFGPara::getNParents, lb);
	rtables.resize(net->size());
	for(auto& rt : rtables)
		rt.init(lb);
}

void StrategyOFGPara::registerAllWorkers()
{
	// broadcast online signal
	rph.input(MType::CReg, net->id()); 
	net->broadcast(MType::CReg, net->id());
	
	// wait until receive all onine signals from all others (using ImmediateHandler)
	int timeToRegister = 5;
	if(!suReg.wait_for(chrono::seconds(timeToRegister))) {
		// error if not finished within XX seconds
		throw runtime_error("[Register Error : " + to_string(net->id()) + "]"
			" Cannot detect all workers within " + to_string(timeToRegister) + " seconds");
	}

	// broadcast starting signal
	rph.input(MType::CReady, net->id());
	net->broadcast(MType::CReady, net->id());
	int timeToReady = 5;
	if(!suStart.wait_for(chrono::seconds(timeToReady))) {
		throw runtime_error("[Start Error : " + to_string(net->id()) + "]"
			" Cannot start all workers within " + to_string(timeToReady) + " seconds");
	}
}

void StrategyOFGPara::initialCE_para()
{
	int size = net->size();
	int id = net->id();
	// split the edge space evenly and check them individually

	int nEdgeInAll = nNode*(nNode - 1) / 2;
	pair<int, int> cef = num2Edge(static_cast<int>(floor(nEdgeInAll / (double)size)*id));
	pair<int, int> cel = num2Edge(static_cast<int>(floor(nEdgeInAll / (double)size)*(id+1)));

	double factor = 1.0 / pgp->size();
	int th = static_cast<int>(ceil(minSup*pgp->size()));
	th = max(th, 1); // in case of minSup=0
	vector<pair<Edge, double>> ceLocal;
	for(int i = cef.first; i <= cel.first; ++i) {
		int j = (i == cef.first ? cef.second : i + 1);
		int endj = (i == cel.first ? cel.second : nNode);
		while(j < endj) {
			int t = countEdgeXSub(i, j, *pgp);
			if(t >= th) {
				auto f = t*factor;
				ceLocal.emplace_back(Edge(i, j), f);
			}
			++j;
		}
	}

	// shared the candidate edges among all workers
	rph.input(MType::CEInit, net->id());
	net->broadcast(MType::CEInit, ceLocal);
	{
		lock_guard<mutex> lg(mce);
		if(edges.empty())
			edges = ceLocal;
		else
			move(ceLocal.begin(), ceLocal.end(), back_inserter(edges));
	}
	suCEinit.wait();
	sort(edges.begin(), edges.end(),
		[](const pair<Edge, double>& lth, const pair<Edge, double>& rth) {
		return lth.second < rth.second; });
}

void StrategyOFGPara::work_para()
{
	// initial the first level
	assignBeginningMotifs();
	int id = net->id();
	int size = net->size();
	// work on the activated motifs
	int ProcessInterval = 10; //milliseconds
	while(!searchFinished) {
		Timer t;
		while(!ltable.emptyActivated()) {
			bool b;
			std::pair<Motif, double> mu;
			tie(b, mu) = ltable.getOne();
			if(!b) {
				break;
			} else if(mu.second < lowerBound) {
				continue;
			}
			pair<vector<Motif>, double> temp = explore(mu.first);
			// TODO: add local lowerbound update logic (change explore logic)
			if(temp.second > lowerBound) {
				for(auto& m : temp.first) {
					generalUpdateCandidateMotif(m, temp.second);
				}
			} else {
				for(auto& m : temp.first)
					generalAbandonCandidateMotif(m);
			}

		}
		for(int i = 0; i < size; i++) {
			if(i == id)
				continue;
			auto vec = rtables[i].collect();
			net->send(i, MType::MNormal, vec);
		}

		int e = ProcessInterval - static_cast<int>(t.elapseMS());
		if(e > 0) {
			this_thread::sleep_for(chrono::milliseconds(e));
		}
	}
}

void StrategyOFGPara::cooridnateTopK()
{
}

void StrategyOFGPara::gatherStatistics()
{
}

void StrategyOFGPara::messageReceiver()
{
	int ProcessInterval = 10; //milliseconds
	string data;
	RPCInfo info;
	info.dest = net->id();
	while(running_ || driver.busy()) {
		Timer t;
		if(net->tryReadAny(data, &info.source, &info.tag)) {
			driver.pushData(move(data), info);
		}
		driver.popData();
		int e = ProcessInterval - static_cast<int>(t.elapseMS());
		if(e > 0) {
			this_thread::sleep_for(chrono::milliseconds(e));
		}
	}
}

std::pair<int, int> StrategyOFGPara::num2Edge(const int idx)
{
	// i, j and idx all start from 0
	if(idx == nNode*(nNode - 1) / 2)
		// the normal look cannot finish for this case
		return make_pair(nNode - 2, nNode - 1);
	int i = 0;
	int n = nNode - i - 1;
	while(idx >= n) {
		++i;
		n += nNode - i - 1;
	}
	int j = idx - n + nNode;
	return make_pair(i, j);
}

int StrategyOFGPara::getMotifOwner(const Motif & m)
{
	return hash<Motif>()(m) % net->size();
}

std::pair<std::vector<Motif>, double> StrategyOFGPara::explore(const Motif& m)
{
	MotifBuilder mb(m);
	double score = scoring(mb, lowerBound);
	if(score == numeric_limits<double>::lowest()) {
		// abandon if not promissing
		return make_pair(vector<Motif>(), score);
	} else if(holder->updatable(score)) {
		holder->update(mb.toMotif(), score);
		//if(flagDCESBound)
		//	maintainDCESBound(edges, holder.lastScore());
	}
	// generate new motifs if not abandon
	vector<Motif> res = expand(m);
	return make_pair(move(res), score);
}

std::vector<Motif> StrategyOFGPara::expand(const Motif & m)
{
	vector<Motif> res;
	MotifBuilder mb(m);
	lock_guard<mutex> lg(mce);
	for(size_t i = 0; i < edges.size(); ++i) {
		const Edge& e = edges[i].first;
		if((mb.containNode(e.s) || mb.containNode(e.d)) && !mb.containEdge(e.s, e.d)) {
			Motif t(m);
			t.addEdge(e.s, e.d);
			res.push_back(move(t));
			//usedEdge[i] = true;
			++stNumMotifGenerated;
		}
	}
	return res;
}
/*
int StrategyOFGPara::getNParents(const MotifBuilder & m)
{
	return quickEstimiateNumberOfParents(m);
}
*/
int StrategyOFGPara::getNParents(const Motif & m)
{
	return quickEstimiateNumberOfParents(m);
}

void StrategyOFGPara::assignBeginningMotifs()
{
	int id = net->id();
	for(auto & ef : edges) {
		Motif m;
		m.addEdge(ef.first.s, ef.first.d);
		int o = getMotifOwner(m);
		if(o == id)
			ltable.update(m, ef.second);
	}
}

void StrategyOFGPara::generalUpdateCandidateMotif(const Motif & m, const double ub)
{
	int o = getMotifOwner(m);
	if(o == net->id()) {
		ltable.update(m, ub);
	} else {
		rtables[o].update(m, ub);
	}
}

void StrategyOFGPara::generalAbandonCandidateMotif(const Motif & m)
{
	static constexpr double WORSTSCORE = numeric_limits<double>::lowest();
	int o = getMotifOwner(m);
	if(o == net->id()) {
		ltable.update(m, WORSTSCORE);
	} else {
		rtables[o].update(m, WORSTSCORE);
	}
}

