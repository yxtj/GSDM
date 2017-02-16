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
	"Usage: " + StrategyOFGPara::name + " <k> <theta> <obj-fun> <alpha> [sd] [net] [dces] [npar] [log] [stat]"
);
const std::string StrategyOFGPara::usageParam(
	"  <k>: [integer] return top-k result\n"
	//	"  <minSup>: [double] the minimum show up probability of a motif among positive subjects\n"
	"  <theta>: [double] the minimum show up probability of a motif among the snapshots of a subject\n"
	"  <obj-fun>: [string] name for the objective function (supprot: diff, margin, ratio)\n"
	"  <alpha>: [double] the penalty factor for the negative frequency\n"
	//"  <dist>: optional [dist/dist-no], default disabled, run in distributed manner\n"
	"  [sd]: optional [sd/sd-no], default enabled, use the shortest distance optimization\n"
	"  [net]: optional [net/net-no], default enabled, use the motif network to prune (a motif's all parents should be valid)\n"
	"  [dces]: optional [dces/dces-c/dces-b/decs-no](:<ms>), "
	"default enabled, use dynamic candidate edge set ('c' for connected-condition, 'b' for bound-condition) "
	"<ms> is the minimum frequency of candidate edges, default 0.0\n"
	"  [npar]: opetional [npar-estim/npar-exact], default estimated, which way used to calculated the number of parents of a motif\n"
	"  [log]: optional [log:<path>/log-no], default disabled, output the score of the top-k result to given path\n"
	"  [stat]: optional [stat:<path>/stat-no], default disabled, dump the statistics of all workers to a file"
);
const std::string StrategyOFGPara::usage = StrategyOFGPara::usageDesc + "\n" + StrategyOFGPara::usageParam;

bool StrategyOFGPara::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 4, 9, name);
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
		flagStatDump = false;
		regex reg_sd("sd(-no)?");
		regex reg_net("net(-no)?");
		regex reg_dces("dces(-[cb])?(-no)?(:0\\.\\d+)?");
		regex reg_log("log(:.+)?(-no)?");
		regex reg_stat("stat(:.+)?(-no)?");
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
			} else if(regex_match(param[i], m, reg_stat)) {
				bool flag = !m[2].matched;
				parseStat(m[1], flag);
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

void StrategyOFGPara::parseStat(const ssub_match & param, const bool flag)
{
	flagStatDump = flag;
	if(flag) {
		pathStatDump = param.str().substr(1);
	}

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
		cout << logHeadID("LOG") + "Generating subject signatures..." << endl;
		Timer timer;
		setSignature();
		cout << logHeadID("LOG") + "  Signatures generated in " + to_string(timer.elapseS()) + " s" << endl;
	}
	running_ = true;
	thread tRecvMsg(bind(&StrategyOFGPara::messageReceiver, this));
	int id = net->id();
	int size = net->size();

	// step 1: register all workers
	registerAllWorkers();
	if(id == MASTER_ID)
		cout << logHead("LOG") + "All workers started." << endl;

	// step 2: initial candidate edge set
	timer.restart();
	initialCE_para();
	if(id == MASTER_ID) {
		cout << logHead("LOG") + "Candidate edges initialized: " + to_string(edges.size()) + " in all." << endl;
		/*stream fout("../data_adhd/try/edge1.txt");
		for(auto& ef : edges) {
			fout << get<1>(ef) << "\t(" << get<0>(ef).s << "," << get<0>(ef).d << ")\n";
		}*/
	}

	// step 3: search independently (each worker holds motif whose score is larger than current k-th)
	work_para();
	if(id == MASTER_ID) {
		cout << logHead("LOG") + "Global search finished." << endl;
	}

	// step 4: coordinate the top-k motifs of all workers
	if(id == MASTER_ID) {
		cout << logHead("LOG") + "Gathering results..." << endl;
	}
	gatherResult();
	vector<Motif> res;
	if(id == MASTER_ID) {
		cout << logHead("LOG") + "Global top-k motifs gathered, "
			+ to_string(holder->size()) + "/" + to_string(k) + " in all, "
			+ "last score=" + to_string(holder->lastScore()) << endl;
		if(flagOutputScore) {
			ofstream fout(pathOutputScore);
			for(auto& p : holder->data) {
				fout << p.second << "\t" << p.first << "\n";
			}
		}
		res = holder->getResultMove();
	}
	delete holder;

	// step 5: merge statistics to the master
	if(id == MASTER_ID) {
		cout << logHead("LOG") + "Gathering Statistics..." << endl;
	}
	st.timeTotal = timer.elapseMS();
	// TODO: change scan functions to use Stat
	st.nGraphChecked += stNumGraphChecked;
	st.nSubjectChecked += stNumSubjectChecked;
	st.nFreqPos += stNumFreqPos;
	st.nFreqNeg += stNumFreqNeg;
	st.netByteSend += net->stat_send_byte;
	st.netByteRecv += net->stat_recv_byte;

	gatherStatistics();
	auto ts = timer.elapseS();
	if(id == MASTER_ID) {
		cout << logHead("LOG") + "Statistical information gathered." << endl;
		cout << "Finished in " << ts << " seconds\n";
		cout << "[Summary on average of " << size << " workers]:\n";
		statFormatOutput(cout, st);
		if(flagStatDump)
			statDump();
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
	globalBound = numeric_limits<decltype(globalBound)>::lowest();
	holder = new TopKBoundedHolder<Motif, double>(k);
	holder->updateBound(globalBound);
	// level 0 starts as finished by its definition
	finishedAtLevel.resize(net->size(), 0);
	lastFinishLevel = &finishedAtLevel[net->id()];
	globalTopKScores.init(k);
}

void StrategyOFGPara::initLRTables()
{
	ltable.init(&StrategyOFGPara::getNParents, globalBound);
	rtables.resize(net->size());
	for(auto& rt : rtables)
		rt.init(globalBound);
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
		throw runtime_error(logHeadID("Register Error") + 
			"Cannot detect all workers within " + to_string(timeToRegister) + " seconds");
	}

	// broadcast starting signal
	rph.input(MType::CReady, net->id());
	net->broadcast(MType::CReady, net->id());
	int timeToReady = 5;
	if(!suStart.wait_for(chrono::seconds(timeToReady))) {
		throw runtime_error(logHeadID("Start Error") +
			"Cannot start all workers within " + to_string(timeToReady) + " seconds");
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

int StrategyOFGPara::getMotifOwner(const Motif & m)
{
	return hash<Motif>()(m) % net->size();
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
		++st.nMotifGenerated;
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
	using T = decltype(holder->lastScore());
	static constexpr double WORSTSCORE = numeric_limits<T>::lowest();
	int o = getMotifOwner(m);
	if(o == net->id()) {
		ltable.update(m, WORSTSCORE);
	} else {
		rtables[o].update(m, WORSTSCORE);
	}
}
