#include "stdafx.h"
#include "StrategyOFGPara.h"
#include "Option.h"
#include "MType.h"
#include "../holder/SDSignature.h"
#include "../util/Timer.h"
#include <regex>

using namespace std;

const std::string StrategyOFGPara::name("ofg-para");
const std::string StrategyOFGPara::usage(
	"Select the discriminative motifs as result.\n"
	"Usage: " + name + " <k> <theta> <obj-fun> [sd] [net] [dces] [npar] [log] [stat]\n"
	"  <k>: [integer] return top-k result\n"
	"  <theta>: [double] the minimum show up probability of a motif among the snapshots of a subject\n"
	"  <obj-fun>: [name:para] name for the objective function (" + ObjFunction::getUsage() + ")\n"
	//"  <dist>: optional [dist/dist-no], default disabled, run in distributed manner\n"
	"  [sd]: optional [sd/sd-no], default enabled, use the shortest distance based signature optimization\n"
	"  [net]: optional [net/net-no], default enabled, generate a new motif only when all parents are promising\n"
	"  [dces]: optional [dces/dces-c/dces-b/decs-no](:<ms>), "
	"default enabled, use dynamic candidate edge set ('c' for connected-condition, 'b' for bound-condition) "
	"<ms> is the minimum frequency of candidate edges, default 0.0\n"
	"  [async]: optional [async/async-no/sync], default enabled, use the asynchronous distribution\n"
	"  [npar]: opetional [npar-estim/npar-exact], default estimated, which way used to calculated the number of parents of a motif\n"
	"  [log]: optional [log:<path>/log-no], default disabled, output the score of the top-k result to given path\n"
	"  [stat]: optional [stat:<path>/stat-no], default disabled, dump the statistics of all workers to a file"
);

bool StrategyOFGPara::parse(const std::vector<std::string>& param)
{
	try {
		checkParam(param, 3, 9, name);
		k = stoi(param[1]);
		pSnap = stod(param[2]);
		parseObj(param[3]);
		flagUseSD = true;
		flagNetworkPrune = true;
		flagDCESConnected = true;
		flagDCESBound = true;
		flagAsync = true;
		minSup = 0.0;
		flagOutputScore = false;
		flagStatDump = false;
		regex reg_sd("sd(-no)?");
		regex reg_net("net(-no)?");
		regex reg_dces("dces(-[cb])?(-no)?(:0\\.\\d+)?");
		regex reg_async("(a)?sync(-no)?");
		regex reg_log("log(:.+)?(-no)?");
		regex reg_stat("stat(:.+)?(-no)?");
		smatch m;
		for(size_t i = 4; i < param.size(); ++i) {
			if(regex_match(param[i], m, reg_sd)) {
				bool flag = !m[1].matched;
				flagUseSD = flag;
			} else if(regex_match(param[i], m, reg_net)) {
				bool flag = !m[1].matched;
				flagNetworkPrune = flag;
			} else if(regex_match(param[i], m, reg_dces)) {
				bool flag = !m[2].matched;
				parseDCES(m[1].str(), m[3].str(), flag);
			} else if(regex_match(param[i], m, reg_async)) {
				bool flag = m[1].matched ^ m[2].matched; // async when only one of (a) and (-no) match
				flagAsync = flag;
			} else if(regex_match(param[i], m, reg_log)) {
				bool flag = !m[2].matched;
				parseLOG(m[1].str(), flag);
			} else if(regex_match(param[i], m, reg_stat)) {
				bool flag = !m[2].matched;
				parseStat(m[1].str(), flag);
			} else {
				throw invalid_argument("Unknown option for strategy " + name + ": " + param[i]);
			}
		}
	} catch(exception& e) {
		cerr << "Cannot finish parse parameters for strategy: " << name << "\n";
		cerr << e.what() << endl;
		return false;
	}
	return true;
}

void StrategyOFGPara::parseObj(const std::string & func_str)
{
	objFun.setFunc(func_str);
}

void StrategyOFGPara::parseDCES(const std::string & option, const std::string & minsup, const bool flag)
{
	if(option.empty()) {
		flagDCESConnected = flagDCESBound = flag;
	} else {
		if(option.find("c") != string::npos) {
			flagDCESConnected = flag;
		} else { //if(opt.find("b") != string::npos)
			flagDCESBound = flag;
		}
	}
	if(!minsup.empty()) {
		minSup = stod(minsup.substr(1));
	}
}

void StrategyOFGPara::parseLOG(const std::string & param, const bool flag)
{
	flagOutputScore = flag;
	if(flag) {
		pathOutputScore = param.substr(1);
	}
}


void StrategyOFGPara::parseStat(const std::string & param, const bool flag)
{
	flagStatDump = flag;
	if(flag) {
		pathStatDump = param.substr(1);
	}

}

std::vector<Motif> StrategyOFGPara::search(
	const Option & opt, DataHolder & dPos, DataHolder & dNeg)
{
	if(!checkInput(dPos, dNeg))
		return std::vector<Motif>();
	// step 0: initialization: parameters, message driver, network thread, message thread
	net = NetworkThread::GetInstance();
	initParams(dPos, dNeg); //should be put after the initialization of net
	initHandlers(); // should be put after the initialization fo net
	running_ = true;
	thread tRecvMsg(bind(&StrategyOFGPara::messageReceiver, this));
	int id = net->id();
	int size = net->size();

	// step 1: register all workers
	registerAllWorkers();
	if(id == MASTER_ID)
		cout << logHead("LOG") + "All workers started." << endl;

	// step 2: generated signature (if required) and initial candidate edge set
	if(flagUseSD) {
		Timer timer;
		if(id == MASTER_ID)
			cout << logHead("LOG") + "Generating subject signatures..." << endl;
		initialSignature_para();
		//if(id == MASTER_ID)
			cout << logHead("LOG") + "  Signatures generated in " + to_string(timer.elapseS()) + " s" << endl;
	}
	timer.restart();
	initialCE_para(dPos);
	if(id == MASTER_ID) {
		cout << logHead("LOG") + "Candidate edges initialized: " + to_string(edges.size()) + " in all." << endl;
		/*stream fout("../data_adhd/try/edge1.txt");
		for(auto& ef : edges) {
			fout << get<1>(ef) << "\t(" << get<0>(ef).s << "," << get<0>(ef).d << ")\n";
		}*/
	}

	// step 3: search independently (each worker holds motif whose score is larger than current k-th)
	if(flagAsync)
		work_para_async();
	else
		work_para_sync();
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
		holder->sort();
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

void StrategyOFGPara::initParams(DataHolder& dPos, DataHolder& dNeg)
{
	dPos.setTheta(pSnap);
	dNeg.setTheta(pSnap);
	nNode = dPos.getnNode();
	pdp = &dPos;
	pdn = &dNeg;
	objFun.setTotalPos(dPos.size());
	objFun.setTotalNeg(dNeg.size());

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

void StrategyOFGPara::initialSignature_para()
{
	initialSignParaOne(*pdp, 1);
	initialSignParaOne(*pdn, 0);
	Timer t;
	suSGInit.wait();
	st.timeWait += t.elapseMS();
}

void StrategyOFGPara::initialSignParaOne(DataHolder & dh, const int dtype)
{
	int size = net->size();
	int id = net->id();
	int n = dh.size();
	int f = n*id / size, l = n*(id + 1) / size;
	dh.initSignature(f, l);
	rph.input(MType::SGInit, id);
	if(size > 1)
		net->broadcast(MType::SGInit, signSerialize(dh, dtype, f, l));
}

bool StrategyOFGPara::signRecv(const std::string & msg)
{
	int dtype, f, l;
	vector<SDSignature> sg;
	tie(dtype, f, l, sg) = signDeserialize(msg);
	if(l - f == sg.size()) {
		for(int i = f; i < l; ++i)
			signMerge(dtype, i, move(sg[i - f]));
		return true;
	}
	return false;
}

std::string StrategyOFGPara::signSerialize(DataHolder & dh, const int dtype, const int f, const int l)
{
	string buffer;
	buffer.resize(3 * sizeof(int32_t) + (l - f)*nNode*nNode * sizeof(int32_t));
	char* p = const_cast<char*>(buffer.data());
	int32_t* pi = reinterpret_cast<int32_t*>(p);
	*pi++ = dtype;
	*pi++ = f;
	*pi++ = l;
	p = reinterpret_cast<char*>(pi);
	for(int i = f; i < l; ++i) {
		SDSignature* ps = dh.getSignature(i);
		p = serialize<SDSignature>(p, *ps);
	}
	return buffer;
}

std::tuple<int, int, int, std::vector<SDSignature>> StrategyOFGPara::signDeserialize(const std::string & msg)
{
	const int32_t* pi = reinterpret_cast<const int32_t*>(msg.data());
	int dtype = *pi++;
	int f = *pi++;
	int l = *pi++;
	const char* p = reinterpret_cast<const char*>(pi);
	vector<SDSignature> sg;
	sg.reserve(l - f);
	for(int i = f; i < l; ++i) {
		auto sp = deserialize<SDSignature>(p);
		p = sp.second;
		sg.push_back(move(sp.first));
	}
	return make_tuple(dtype, f, l, move(sg));
}

void StrategyOFGPara::signMerge(const int dtype, const int idx, SDSignature && sd)
{
	getDataHolder(dtype)->setSignature(idx, move(sd));
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

DataHolder * StrategyOFGPara::getDataHolder(const int dtype)
{
	return dtype == 1 ? pdp : pdn;
	if(dtype == 1)
		return pdp;
	else
		return pdn;
}

int StrategyOFGPara::getMotifOwner(const Motif & m)
{
	return hash<Motif>()(m) % net->size();
}

int StrategyOFGPara::quickEstimateNumberOfParents(const Motif & m)
{
	unordered_map<int, int> cont;
	for(auto&e : m.edges) {
		++cont[e.s];
		++cont[e.d];
	}
	return count_if(cont.begin(), cont.end(),
		[](const pair<const int, int>& p) {return p.second == 1; });
}

/*
int StrategyOFGPara::getNParents(const MotifBuilder & m)
{
	return quickEstimateNumberOfParents(m);
}
*/
int StrategyOFGPara::getNParents(const Motif & m)
{
	return quickEstimateNumberOfParents(m);
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
