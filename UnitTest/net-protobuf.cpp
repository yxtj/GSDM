#include "stdafx.h"
#include "../net-protobuf/NetworkThread.h"
#include "msg/common.pb.h"

using namespace std;

static void workRank0(NetworkThread *net) {
	int rank = 0;
	cout << "in work rank " << rank << endl;
	Arg ag_apple;
	ag_apple.set_key("apple");
	ag_apple.set_value("2 pieces");
	cout << "r" << rank << " send apple" << endl;
	net->send(1, 0, ag_apple);

	string buffer;
	net->readAny(buffer);
	Arg ag_n;
	ag_n.ParseFromString(buffer);
	cout << "r" << rank << " received pear: " << ag_n.key() << " : " << ag_n.value() << endl;
	
	Args content;
	content.add_param()->CopyFrom(ag_apple);
	content.add_param()->CopyFrom(ag_n);
	cout << "r" << rank << " send fruits" << endl;
	net->send(1, 1, content);

	net->readAny(buffer);
	Args rcvCont;
	rcvCont.ParseFromString(buffer);

	cout << rank << " received fruits: " << endl;
	for(int i = 0; i < rcvCont.param_size(); ++i) {
		auto& t = rcvCont.param(i);
		cout << "r" << rank << "-" << i << " : " << t.key() << " " << t.value() << "\t";
	}
	cout << endl;
}

static void workRank1(NetworkThread *net) {
	int rank = 1;
	cout << "in work rank " << rank << endl;
	Arg ag_pear;
	ag_pear.set_key("pear");
	ag_pear.set_value("4 dollar");
	cout << "r" << rank << " send pear" << endl;
	net->send(0, 0, ag_pear);

	string buffer;
	net->readAny(buffer);
	Arg ag_n;
	ag_n.ParseFromString(buffer);
	cout << "r" << rank << " received apple: " << ag_n.key() << " : " << ag_n.value() << endl;

	Args content;
	content.add_param()->CopyFrom(ag_pear);
	content.add_param()->CopyFrom(ag_n);
	cout << "r" << rank << " send fruits" << endl;
	net->send(0, 1, content);

	net->readAny(buffer);
	Args rcvCont;
	rcvCont.ParseFromString(buffer);

	cout << rank << " received fruits: " << endl;
	for(int i = 0; i < rcvCont.param_size(); ++i) {
		auto& t = rcvCont.param(i);
		cout << "r" << rank << "-" << i << " : " << t.key() << " " << t.value() << "\t";
	}
	cout << endl;
}

void testNetProtoBuf(int argc, char* argv[]) {
	NetworkThread::Init(argc, argv);
	NetworkThread *net = NetworkThread::GetInstance();
	cout << net->id() << " / " << net->size() << endl;
	if(net->size() == 1) {
		cerr << "should be run with more than one mpi instance" << endl;
		return;
	}

	if(net->id() == 0) {
		workRank0(net);
	} else if(net->id() == 1) {
		workRank1(net);
	}
	
}
