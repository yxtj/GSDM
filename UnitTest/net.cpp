#include "stdafx.h"
#include "declare.h"
#include "../net/NetworkThread.h"

using namespace std;

static int SYNC_TAG = 0;

ostream& printSend(int rank, int tag, int size) {
	return cout << "  r=" << rank << ", send tag=" << tag << ", size=" << size;
}
ostream& printReceive(int rank, int tag, int size) {
	return cout << "  r=" << rank << ", recv tag=" << tag << ", size=" << size;
}

static void workRank0(NetworkThread *net) {
	int rank = 0;
	int DST = 1;
	cout << "in work rank " << rank << endl;

	int a = 1;
	double b = 2.0;
	pair<int, int> c(3, 4);
	Motif d;	d.addEdge(5, 6);
	Motif d2;	d2.addEdge(5, 6); d2.addEdge(15,16);
	vector<int> e{ 7,8,9 };
	map<Motif, double> f;	f[d] = 0.1;	f[d2] = 0.2;

	int size;
	int source, tag;
	string buff;

	// simple unit
	size = net->send(DST, 1, a);
	printSend(rank, 1, size) << ", value=" << a << endl;
	size = net->send(DST, 2, b);
	printSend(rank, 2, size) << ", value=" << b << endl;
	size = net->send(DST, 3, c);
	printSend(rank, 1, size) << ", value=" << c << endl;

	net->readAny(buff, &source, &tag);
	printReceive(rank, tag, buff.size()) << ", value=" << deserialize<char>(buff) << endl;

	// Motif
	size = net->send(DST, 4, d);
	printSend(rank, 4, size) << ", value=" << d << endl;

	net->readAny(buff, &source, &tag);
	printReceive(rank, tag, buff.size()) << ", value=" << deserialize<char>(buff) << endl;

	// container
	size = net->send(DST, 5, e);
	printSend(rank, 5, size) << ", value=" << e << endl;
	size = net->send(DST, 6, f);
	printSend(rank, 6, size) << ", value=" << f << endl;

	net->readAny(buff, &source, &tag);
	printReceive(rank, tag, buff.size()) << ", value=" << deserialize<char>(buff) << endl;
}

static void workRank1(NetworkThread *net) {
	int rank = 1;
	cout << "in work rank " << rank << endl;
	int DST = 0;

	int size;
	int source, tag;
	string buff;

	// simple unit
	net->readAny(buff, &source, &tag);
	printReceive(rank, tag, buff.size()) << ", value=" << deserialize<int>(buff) << endl;
	net->readAny(buff, &source, &tag);
	printReceive(rank, tag, buff.size()) << ", value=" << deserialize<double>(buff) << endl;
	net->readAny(buff, &source, &tag);
	printReceive(rank, tag, buff.size()) << ", value=" << deserialize<pair<int, int>>(buff) << endl;

	size = net->send(DST, SYNC_TAG, 'x');
	printSend(rank, SYNC_TAG, size) << ", value=" << 'x' << endl;

	// Motif
	net->readAny(buff, &source, &tag);
	printReceive(rank, tag, buff.size()) << ", value=" << deserialize<Motif>(buff) << endl;

	size = net->send(DST, SYNC_TAG, 'y');
	printSend(rank, SYNC_TAG, size) << ", value=" << 'y' << endl;

	// container
	net->readAny(buff, &source, &tag);
	printReceive(rank, tag, buff.size()) << ", value=" << deserialize<vector<int>>(buff) << endl;
	net->readAny(buff, &source, &tag);
	printReceive(rank, tag, buff.size()) << ", value=" << deserialize<map<Motif, double>>(buff) << endl;

	size = net->send(DST, SYNC_TAG, 'z');
	printSend(rank, SYNC_TAG, size) << ", value=" << 'z' << endl;
}

void testNet(int argc, char* argv[]) {
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
