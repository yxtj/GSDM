#include "stdafx.h"
#include <sstream>
#include "../serialization/serialization.h"

using namespace std;

static string printHex(char* f, char* l) {
	ostringstream oss;
	oss << "0x";
	while(f < l) {
		char ch = *f;
		int v = ch;
		if(v < 15)
			oss << '0';
		oss << hex << v;
		++f;
	}
	return oss.str();
}

static void test_single1() {
	int a = 7;
	char buf[128];
	char* p = buf;
	char* p1=serialize(p, a);
	cout << "input:  " << a << "\tbytes: " << p1 - p << endl;
	cout << "serialized data: " << printHex(p, p1) << endl;
	auto res = deserialize<int>(p);
	char* p2 = res.second;
	cout << "output: " << res.first << "\tbytes: " << p2 - p << endl;
}

static void test_single2() {
	double a = 1.2;
	char buf[128];
	char* p = buf;
	char* p1 = serialize(p, a);
	cout << "input:  " << a << "\tbytes: " << p1 - p << endl;
	cout << "serialized data: " << printHex(p, p1) << endl;
	auto res = deserialize<int>(p);
	char* p2 = res.second;
	cout << "output: " << res.first << "\tbytes: " << p2 - p << endl;
}

static void test_pair() {
	pair<int, double> a(3, 1.2);
	char buf[128];
	char *p = buf;
	char *p1 = serialize(p, a);
	cout << "input:  " << a.first << " , " << a.second << "\tbytes: " << p1 - p << endl;
	cout << "serialized data: " << printHex(p, p1) << endl;
	auto res = deserialize<pair<int,double>>(p);
	char* p2 = res.second;
	cout << "output: " << res.first.first << " , " << res.first.second << "\tbytes: " << p2 - p << endl;
}

static void test_motif() {
	Motif m;
	m.addEdge(1, 2);
	m.addEdge(2, 5);
	m.addEdge(5, 15);
	m.addEdge(15, 7);

	char buf[128];
	char *p = buf;
	char *p1 = serialize(p, m);
	cout << "input:  " << m << "\tbytes: " << p1 - p << endl;
	cout << "serialized data: " << printHex(p, p1) << endl;
	auto res = deserialize<Motif>(p);
	char* p2 = res.second;
	cout << "output: " << res.first << "\tbytes: " << p2 - p << endl;
}

static void test_vector() {
	vector<int> v{1,2,3,5,7,11};

	char buf[128];
	char *p = buf;
	auto sr = serializeCont<vector<int>>(p, 128, v.cbegin(), v.cend());
	char *p1 = sr.first;
	cout << "input:  [";
	for(auto t : v) cout << t << " ";
	cout << "]\tbytes: " << p1 - p << "\tnum: " << sr.second - v.cbegin() << endl;
	cout << "serialized data: " << printHex(p, p1) << endl;
	auto res = deserializeCont<vector<int>>(p);
	char* p2 = res.second;
	cout << "output: [";
	for(auto t : res.first) cout << t << " ";
	cout << "]\tbytes: " << p2 - p << "\tnum: " << res.first.size() << endl;
}

static void test_map() {
	map<int, int> m{ {1,2},{3,6},{7,14},{5,10} };

	char buf[128];
	char *p = buf;
	auto sr = serializeCont<map<int,int>>(p, 128, m.cbegin(), m.cend());
	char *p1 = sr.first;
	cout << "input:  [";
	for(auto t : m) cout << t.first<<":"<<t.second << " ";
	cout << "]\tbytes: " << p1 - p << "\tnum: " << distance(m.cbegin(), sr.second) << endl;
	cout << "serialized data: " << printHex(p, p1) << endl;
	auto res = deserializeCont<map<int,int>>(p);
	char* p2 = res.second;
	cout << "output: [";
	for(auto t : res.first) cout << t.first << ":" << t.second << " ";
	cout << "]\tbytes: " << p2 - p << "\tnum: " << res.first.size() << endl;
}

static void test_vm() {
	Motif m1; m1.addEdge(1, 2); m1.addEdge(2, 4);
	Motif m2; m2.addEdge(2, 3); m2.addEdge(3, 6);
	vector<Motif> v{ m1,m2 };

	char buf[128];
	char *p = buf;
	auto sr = serializeCont<vector<Motif>>(p, 128, v.cbegin(), v.cend());
	char *p1 = sr.first;
	cout << "input:  [";
	for(auto t : v) cout << t << " ";
	cout << "]\tbytes: " << p1 - p << "\tnum: " << sr.second - v.cbegin() << endl;
	cout << "serialized data: " << printHex(p, p1) << endl;
	auto res = deserializeCont<vector<Motif>>(p);
	char* p2 = res.second;
	cout << "output: [";
	for(auto t : res.first) cout << t << " ";
	cout << "]\tbytes: " << p2 - p << "\tnum: " << res.first.size() << endl;
}

void testSerialzation(int arg, char* argv[]) {
	cout << "  testing single int" << endl;
	test_single1();
	cout << "  testing single double" << endl;
	test_single2();
	
	cout << "  testing pair<int,double>" << endl;
	test_pair();

	cout << "  testing motif" << endl;
	test_motif();

	cout << "  testing vector" << endl;
	test_vector();

	cout << "  testing map" << endl;
	test_map();

	cout << "  testing vector<Motif> (existing overloading function)" << endl;
	test_vm();
}
