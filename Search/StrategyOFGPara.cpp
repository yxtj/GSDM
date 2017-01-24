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
	if(flagUseSD) {
		ostringstream oss;
		oss << "[LOG : " << net->id() << "] Generating subject signatures...";
		cout << oss.str() << endl;
		Timer timer;
		setSignature();
		oss.clear();
		oss << "[LOG: " << net->id() << "]   Signatures generated in " << timer.elapseS() << " s" << endl;
		cout << oss.str() << endl;
	}
	running_ = true;
	thread tRecvMsg(bind(&StrategyOFGPara::messageReceiver, this));

	// step 1: register all workers
	registerAllWorkers();
	if(net->id() == MASTER_ID)
		cout << "[LOG] All workers started." << endl;

	// step 2: initial candidate edge set
	Timer t;
	initialCE_para();
	if(net->id() == MASTER_ID)
		cout << "[LOG] Candidate edges initialized: " << edges.size() << " in all." << endl;

	// step 3: search independently (each worker holds motif whose score is larger than current k-th)
	work_para();
	{
		ostringstream oss;
		oss << "[LOG : " << net->id() << "] Finish searching.\n";
		cout << oss.str();
		cout.flush();
	}

	// step 4: coordinate the top-k motifs of all workers
	gatherResult();
	if(net->id() == MASTER_ID) {
		cout << "[LOG] Global top-k motifs gathered." << endl;
	}
	vector<Motif> res = holder->getResultMove();
	delete holder;

	// step 5: merge statistics to Rank 0
	gatherStatistics();
	auto ts = t.elapseS();
	if(net->id() == MASTER_ID) {
		cout << "[LOG] Statistical information gathered." << endl;
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
	lastFinishLevel = 0;
	nFinishLevel.resize(16, 0);
}

void StrategyOFGPara::initLRTables()
{
	double lb = get<1>(edges.back());
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
	vector<tuple<Edge, double, int>> ceLocal;
	for(int i = cef.first; i <= cel.first; ++i) {
		int j = (i == cef.first ? cef.second : i + 1);
		int endj = (i == cel.first ? cel.second : nNode);
		while(j < endj) {
			int t = countEdgeXSub(i, j, *pgp);
			if(t >= th) {
				auto f = t*factor;
				ceLocal.emplace_back(Edge(i, j), f, 0);
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
		[](const tuple<Edge, double, int>& lth, const tuple<Edge, double, int>& rth) {
		return get<1>(lth) < get<1>(rth); });
}

void StrategyOFGPara::work_para()
{
	// initial the first level
	assignBeginningMotifs();
	lastFinishLevel = 0;
	net->broadcast(MType::GLevelFinish, 0); // level 0 starts as finished by definition
	int id = net->id();
	int size = net->size();
	// work on the activated motifs
	Timer twm; // timer for waiting motifs
	Timer tct; // timer for coordinate top-k
	while(!checkSearchFinish()) {
		Timer t;
		// process all activated motifs
		while(!ltable.emptyActivated()) {
			bool b;
			std::pair<Motif, double> mu;
			tie(b, mu) = ltable.getOne();
			if(!b) {
				break;
			} else if(mu.second < lowerBound) {
				continue;
			}
			if(explore(mu.first)) {
				updateThresholdCE(lowerBound);
				if(static_cast<int>(twm.elapseMS()) > INTERVAL_UPDATE_WAITING_MOTIFS) {
					updateThresholdWaitingMotifs(lowerBound);
					twm.restart();
				}
			}
		}
		// send remote table buffers
		for(int i = 0; i < size; i++) {
			if(i == id)
				continue;
			auto vec = rtables[i].collect();
			net->send(i, MType::MNormal, vec);
		}
		// send level finish signal if possible
		processLevelFinish();
		// get the global top-k if necessary
		if(static_cast<int>(tct.elapseMS()) > INTERVAL_COORDINATE_TOP_K) {
			topKCoordinate();
			tct.restart();
		}

		int e = INTERVAL_PROCESS - static_cast<int>(t.elapseMS());
		if(e > 0) {
			this_thread::sleep_for(chrono::milliseconds(e));
		}
	}
	// send search finish signal
	rph.input(MType::GSearchFinish, id);
	net->broadcast(MType::GSearchFinish, lastFinishLevel);
	suSearchEnd.wait();
}

void StrategyOFGPara::gatherResult()
{
	if(net->id() == 0) {
		resultReceive();
	} else {
		resultSend();
	}
}

void StrategyOFGPara::gatherStatistics()
{
	if(net->id() == 0) {
		statReceive();
	} else {
		statSend();
	}
}

void StrategyOFGPara::messageReceiver()
{
	string data;
	RPCInfo info;
	info.dest = net->id();
	while(running_ || driver.busy()) {
		Timer t;
		if(net->tryReadAny(data, &info.source, &info.tag)) {
			driver.pushData(move(data), info);
		}
		driver.popData();
		int e = INTERVAL_PROCESS - static_cast<int>(t.elapseMS());
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

bool StrategyOFGPara::explore(const Motif & m)
{
	bool used = false;
	MotifBuilder mb(m);
	double ub, score;
	tie(ub, score) = scoring(mb, lowerBound);
	// score == numeric_limits<double>::lowest() if the upper bound is not promising
	if(holder->updatable(score)) {
		holder->update(m, score);
		lowerBound = holder->lastScore();
		used = true;
	}
	if(used) {
		for(auto& m : expand(m, used)) {
			generalUpdateCandidateMotif(m, ub);
		}
	} else {
		for(auto& m : expand(m, used))
			generalAbandonCandidateMotif(m);
	}
	return used;
}

std::vector<Motif> StrategyOFGPara::expand(const Motif & m, const bool used)
{
	vector<Motif> res;
	MotifBuilder mb(m);
	lock_guard<mutex> lg(mce);
	for(size_t i = 0; i < edges.size(); ++i) {
		const Edge& e = get<0>(edges[i]);
		if((mb.containNode(e.s) || mb.containNode(e.d)) && !mb.containEdge(e.s, e.d)) {
			Motif t(m);
			t.addEdge(e.s, e.d);
			res.push_back(move(t));
			// TODO: update the last-used-level field of edges if used is set
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
		auto& e = get<0>(ef);
		m.addEdge(e.s, e.d);
		int o = getMotifOwner(m);
		if(o == id)
			ltable.update(m, get<1>(ef));
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

bool StrategyOFGPara::processLevelFinish()
{
	bool moved = false;
	int oldlfl = lastFinishLevel;
	while(checkLevelFinish(lastFinishLevel + 1)) {
		ltable.sortUp(lastFinishLevel + 1);
		lock_guard<mutex> lg(mfl);
		++nFinishLevel[lastFinishLevel + 1]; // local finish
		++lastFinishLevel;
		moved = true;
	}
	if(moved) {
		moveToNewLevel(oldlfl);
		for(int i = oldlfl; i < lastFinishLevel + 1; ++i)
			net->broadcast(MType::GLevelFinish, i);
	}
	return moved;
}

void StrategyOFGPara::moveToNewLevel(const int from)
{
	if(flagDCESConnected) {
		edgeUsageSend(from);
		removeUnusedEdges();
	}
}

bool StrategyOFGPara::checkLevelFinish(const int level)
{
	if(static_cast<int>(nFinishLevel.size()) <= level) {
		lock_guard<mutex> lg(mfl);
		nFinishLevel.resize(max<size_t>(nFinishLevel.size(), level + 1), 0);
		return false;
	}
	if(net->size() == nFinishLevel[level - 1]) {
		if(ltable.emptyActivated(level)) {
			return true;
		}
	}
	return false;
}

bool StrategyOFGPara::checkSearchFinish()
{
	// condition: normal messages arrive before level-finish message
	return net->size() == nFinishLevel[lastFinishLevel]
		&& ltable.getNumEverActive(lastFinishLevel + 1) == 0;
}

void StrategyOFGPara::edgeUsageSend(const int since)
{
	vector<pair<Edge, int>> update;
	update.reserve(edges.size());
	{
		lock_guard<mutex> lg(mce);
		for(auto& et : edges) {
			// mark > since
			if(get<2>(et) >= since) {
				update.emplace_back(get<0>(et), get<2>(et));
			}
		}
	}
	net->broadcast(MType::CEUsage, update);
}

void StrategyOFGPara::edgeUsageUpdate(const std::vector<std::pair<Edge, int>>& usage)
{
	auto fu = usage.begin(), lu = usage.end();
	lock_guard<mutex> lg(mce);
	auto fe = edges.begin(), le = edges.end();
	// rationale: edges and usage are sorted with 
	while(fe != le && fu != lu) {
		if(fu->first < get<0>(*fe)) {
			++fu;
		} else if(get<0>(*fe) < fu->first) {
			++fe;
		} else {
			get<2>(*fe) = max(get<2>(*fe), fu->second);
			++fe;
			++fu;
		}
	}
}

void StrategyOFGPara::removeUnusedEdges()
{
	lock_guard<mutex> lg(mce);
	auto it = remove_if(edges.begin(), edges.end(),
		[=](const tuple<Edge, double, int>& t) {
		return get<2>(t) < lastFinishLevel;
	});
	edges.erase(it, edges.end());
}


void StrategyOFGPara::topKCoordinate()
{
	if(net->id() != MASTER_ID) {
		net->send(MASTER_ID, MType::GGatherLocalTopK, holder->getScore());
		// the rest work will be done by cbRecvTopScore() and topKCoordinateFinish()
	} else {
		lock_guard<mutex> lg(mgtk);
		topKMerge(holder->getScore(), net->id());
	}
}

void StrategyOFGPara::topKCoordinateFinish()
{
	rph.resetTypeCondition(MType::GGatherLocalTopK);
	net->broadcast(MType::GLowerBound, lowerBound);
	updateThresholdCE(lowerBound);
	updateThresholdResult(lowerBound);
	updateThresholdWaitingMotifs(lowerBound);
}

void StrategyOFGPara::topKMerge(const std::vector<double>& recv, const int source)
{
	vector<pair<double, int>> temp;
	auto it = back_inserter(temp);
	int cnt = 0;
	lock_guard<mutex> lg(mgtk);
	auto first1 = globalTopKScores.begin(), last1 = globalTopKScores.end();
	auto first2 = recv.begin(), last2 = recv.end();
	// replace the entries with the same source & sort up
	//   Implemented by ignoring the entries in gTopKScoures with the same source
	while(first1 != last1 && first2 != last2 && cnt <= k) {
		if(first1->second == source) {
			++first1;
		} else if(first1->first <= *first2) {
			*it++ = *first1++;
			++cnt;
		} else {
			*it++ = make_pair(*first2++, source);
			++cnt;
		}
	}
	while(cnt <= k && first1 != last1) {
		*it++ = *first1++;
		++cnt;
	}
	while(cnt <= k && first2 != last2) {
		*it++ = make_pair(*first2++, source);
		++cnt;
	}
	globalTopKScores = move(temp);
	lowerBound = max(lowerBound, globalTopKScores.back().first);
}

void StrategyOFGPara::resultSend()
{
	vector<pair<Motif, double>> topk = holder->getResultScore();
	net->send(MASTER_ID, MType::MGather, topk);
}

void StrategyOFGPara::resultReceive()
{
	rph.input(MType::MGather, net->id());
	suTKGather.wait();
	suTKGather.reset();

}

void StrategyOFGPara::statSend()
{
	vector<unsigned long long> stat = {
		stNumMotifExplored,
		stNumMotifGenerated,
		stNumGraphChecked,
		stNumSubjectChecked,
		stNumFreqPos,
		stNumFreqNeg
	};
	net->send(MASTER_ID, MType::SGather, stat);
}

void StrategyOFGPara::statReceive()
{
	rph.input(MType::SGather, net->id());
	suStat.wait();
}

