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

std::vector<Motif> StrategyOFGPara::search(const Option & opt,
	const std::vector<std::vector<Graph>>& gPos, const std::vector<std::vector<Graph>>& gNeg)
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
	endAtLevel.resize(net->size(), numeric_limits<int>::max());
}

void StrategyOFGPara::initLRTables()
{
	ltable.init(&StrategyOFGPara::getNParents, lowerBound);
	rtables.resize(net->size());
	for(auto& rt : rtables)
		rt.init(lowerBound);
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


void StrategyOFGPara::work_para()
{
	// initial the first level
	initLRTables();
	initLowerBound();
	assignBeginningMotifs();

	int id = net->id();
	int size = net->size();

	lastFinishLevel = 0;
	nFinishLevel[0] = size;
	net->broadcast(MType::GLevelFinish, 0); // level 0 starts as finished by definition

	// work on the activated motifs
	Timer twm; // timer for updating the bound for waiting motifs
	Timer tct; // timer for coordinating global top-k
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
			// update lower bound
			if(explore(mu.first) && holder->size() == k) {
				bool modifyTables = false;
				if(static_cast<int>(twm.elapseMS()) > INTERVAL_UPDATE_WAITING_MOTIFS) {
					modifyTables = true;
					twm.restart();
				}
				updateLowerBound(holder->lastScore(), modifyTables, true);
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
	// send level finish signal if not send before
	processLevelFinish();
	// send search finish signal
	rph.input(MType::GSearchFinish, id);
	net->broadcast(MType::GSearchFinish, lastFinishLevel);
	suSearchEnd.wait();
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
		lock_guard<mutex> lg(mtk);
		holder->update(m, score);
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
	int l = m.getnEdge();
	lock_guard<mutex> lg(mce);
	for(size_t i = 0; i < edges.size(); ++i) {
		const Edge& e = get<0>(edges[i]);
		if((mb.containNode(e.s) || mb.containNode(e.d)) && !mb.containEdge(e.s, e.d)) {
			Motif t(m);
			t.addEdge(e.s, e.d);
			res.push_back(move(t));
			// update the last-used-level field of edges if used is set
			if(used)
				get<2>(edges[i]) = l;
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
		if(o == id) {
			ltable.addToActivated(m, get<1>(ef));
		}
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
	cout << "[LOG : " + to_string(net->id()) + "] Moved from level " 
		+ to_string(from) + " to new level " + to_string(lastFinishLevel) << endl;
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
	// TODO: use moveToNewLevel() to merge this into nFinishLevel
	int ec = count_if(endAtLevel.begin(), endAtLevel.end(), [=](const int l) {
		return l < level;
	});
	if(net->size() == ec + nFinishLevel[level - 1]) {
		if(ltable.emptyActivated(level)) {
			return true;
		}
	}
	return false;
}

bool StrategyOFGPara::checkSearchFinish()
{
	// guarantee: normal messages arrive before level-finish message
	// TODO: use moveToNewLevel() to merge this into nFinishLevel
	int ec = count_if(endAtLevel.begin(), endAtLevel.end(), [=](const int l) {
		return l < lastFinishLevel;
	});
	return net->size() == ec + nFinishLevel[lastFinishLevel]
		&& ltable.getNumEverActive(lastFinishLevel + 1) == 0;
}
