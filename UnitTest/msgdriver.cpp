#include "stdafx.h"
#include "declare.h"
#include <functional>
#include <sstream>
#include "../msgdriver/MsgDriver.h"
#include "../msgdriver/tools/ReplyHandler.h"
#include "../msgdriver/tools/SyncUnit.h"

using namespace std;

static void df(const string& data, const RPCInfo& info) {
	ostringstream oss;
	oss << "[default, " << info.tag << "] data size=" << data.size();
	cout << oss.str() << endl;
}


class TesterMD {
	mutex m_d;
	MsgDriver driver;

	SyncUnit su_reg;
	ReplyHandler rph;

	bool running;

private:
	static const int tpReg = 1;
	void cbReg(const string& data, const RPCInfo& info) {
		ostringstream oss;
		oss << "[reg, " << info.tag << "] from " << info.source << ", data: " << stoi(data) << "\n";
		cout << oss.str();
		cout.flush();
		rph.input(tpReg, info.source - 1);
	}
	static const int tpStop = 2;
	void cbStop(const string&, const RPCInfo& info) {
		ostringstream oss;
		oss << "[stop, " << info.tag << "] from " << info.source << "\n";
		cout << oss.str();
		cout.flush();
		rph.input(tpStop, info.source);
	}
	static const int tpMsg = 3;
	void cbMsg(const string& data, const RPCInfo& info) {
		ostringstream oss;
		oss << "[msg, " << info.tag << "] from " << info.source << ", value: " << data << "\n";
		cout << oss.str();
		cout.flush();
	}

public:
	void init() {
		running = true;
		using namespace placeholders;
		driver.registerDefaultOutHandler(df);
		driver.registerImmediateHandler(tpReg, bind(&TesterMD::cbReg, this, _1, _2));
		driver.registerProcessHandler(tpStop, bind(&TesterMD::cbStop, this, _1, _2));
		driver.registerProcessHandler(tpMsg, bind(&TesterMD::cbMsg, this, _1, _2));

		rph.addType(tpReg, ReplyHandler::condFactory(ReplyHandler::EACH_ONE, 2),
			bind(&SyncUnit::notify, &su_reg));
		//rph.activateType(tpReg);
		rph.addType(tpStop, ReplyHandler::condFactory(ReplyHandler::ANY_ONE),
			bind([&] {running = false; }));
		//rph.activateType(tpStop);
	}

	void go() {
		thread tr(bind(&TesterMD::tmd_receiver, this));
		thread ts1(bind(&TesterMD::tmd_sender1, this));
		thread ts2(bind(&TesterMD::tmd_sender2, this));
		tr.join();
		ts1.join();
		ts2.join();
	}

	void tmd_receiver();
	void tmd_sender1();
	void tmd_sender2();
};

void TesterMD::tmd_receiver()
{
	while(running && driver.busy()) {
		{
			//lock_guard<mutex> lg(m_d);
			driver.popData();
		}
		this_thread::sleep_for(chrono::milliseconds(10));
	}

}

void TesterMD::tmd_sender1()
{
	int id = 1;
	RPCInfo info;
	info.source = id;
	info.dest = 0;

	info.tag = tpReg;
	{
		//lock_guard<mutex> lg(m_d);
		driver.pushData(to_string(id), info);
	}
	cout << "s-" << id << " waiting for su_reg" << endl;
	su_reg.wait();

	cout << "s-" << id << " sending messages" << endl;
	info.tag = tpMsg;
	{
		//lock_guard<mutex> lg(m_d);
		driver.pushData("apple", info);
	}
	{
		//lock_guard<mutex> lg(m_d);
		driver.pushData("banana", info);
	}
}
void TesterMD::tmd_sender2()
{
	int id = 2;
	RPCInfo info;
	info.source = id;
	info.dest = 0;

	info.tag = tpReg;
	{
		//lock_guard<mutex> lg(m_d);
		driver.pushData(to_string(id), info);
	}
	cout << "s-" << id << " waiting for su_reg" << endl;
	su_reg.wait();

	cout << "s-" << id << " sending messages" << endl;
	info.tag = tpMsg;
	{
		//lock_guard<mutex> lg(m_d);
		driver.pushData("pear", info);
	}
	{
		//lock_guard<mutex> lg(m_d);
		driver.pushData("orange", info);
	}

	info.tag = tpStop;
	cout << "s-" << id << " send stop signal" << endl;
	{
		//lock_guard<mutex> lg(m_d);
		driver.pushData("", info);
	}
}

void testMsgDriver(int argc, char* argv[]) {
	TesterMD tmd;
	tmd.init();
	tmd.go();
}

