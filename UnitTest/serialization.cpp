#include "stdafx.h"
#include <sstream>
#include "../serialization/serialization.h"

using namespace std;

static string printHex(const char* f, const char* l) {
	ostringstream oss;
	oss << "0x";
	while(f < l) {
		char ch = *f;
		int v = ch;
		if(v <= 15)
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
	const char* p2 = res.second;
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
	const char* p2 = res.second;
	cout << "output: " << res.first << "\tbytes: " << p2 - p << endl;
}

static void test_single3() {
	string a("hello world");
	char buf[128];
	char* p = buf;
	char* p1 = serialize(p, a);
	cout << "input:  " << a << "\tbytes: " << p1 - p << endl;
	cout << "serialized data: " << printHex(p, p1) << endl;
	auto res = deserialize<string>(p);
	const char* p2 = res.second;
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
	const char* p2 = res.second;
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
	const char* p2 = res.second;
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
	auto res = deserialize<vector<int>>(p);
	const char* p2 = res.second;
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
	auto res = deserialize<map<int,int>>(p);
	const char* p2 = res.second;
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
	auto res = deserialize<vector<Motif>>(p);
	const char* p2 = res.second;
	cout << "output: [";
	for(auto t : res.first) cout << t << " ";
	cout << "]\tbytes: " << p2 - p << "\tnum: " << res.first.size() << endl;
}

void test_ummp() {
	Motif m1; m1.addEdge(1, 2); m1.addEdge(2, 4);
	Motif m2; m2.addEdge(2, 3); m2.addEdge(3, 6);
	unordered_map<Motif, pair<int, double>> cont{ {m1,{3,0.3}},{m2,{5,0.5}} };
	
	char buf[128];
	char *p = buf;
	auto sr = serializeCont<unordered_map<Motif, pair<int, double>>>(p, 128, cont.cbegin(), cont.cend());
	char *p1 = sr.first;
	cout << "input:  [";
	for(auto& t : cont) cout << t.first << ":(" << t.second.first<<","<<t.second.second << ") ";
	cout << "]\tbytes: " << p1 - p << "\tnum: " << distance(cont.cbegin(), sr.second) << endl;
	cout << "serialized data: " << printHex(p, p1) << endl;
	auto res = deserialize<unordered_map<Motif, pair<int, double>>>(p);
	const char* p2 = res.second;
	cout << "output: [";
	for(auto t : res.first) cout << t.first << ":(" << t.second.first << "," << t.second.second << ") ";
	cout << "]\tbytes: " << p2 - p << "\tnum: " << res.first.size() << endl;
}

void test_tostring() {
	int a = 6;
	char buf[128];
	char* p = serialize(buf, a);
	string str = serialize(a);
	cout << "char *: " << printHex(buf, p) << endl;
	cout << "string: " << printHex(str.data(), str.data() + str.size()) << endl;
	auto res_a = deserialize<int>(str.data());
	cout << "value: " << res_a.first << endl;

	vector<int> v{ 1,3,5,7 };
	p = serialize(buf, v);
	str = serialize(v);
	cout << "char *: " << printHex(buf, p) << endl;
	cout << "string: " << printHex(str.data(), str.data() + str.size()) << endl;
	auto res_v = deserialize<vector<int>>(str.data());
	cout << "value: [";
	for(auto temp : res_v.first) cout << temp << " ";
	cout << "]" << endl;
}

void test_fromstring() {
	int a = 123;
	string s = serialize(a);
	cout << "input:  " << a << "\tbytes: " << s.size() << endl;
	cout << "serialized data: " << printHex(s.data(), s.data() + s.size()) << endl;
	int ra = deserialize<int>(s);
	cout << "output: " << ra << endl;
}

void testSerialzation(int arg, char* argv[]) {
	cout << "  testing single int" << endl;
	test_single1();
	cout << "  testing single double" << endl;
	test_single2();
	cout << "  testing single string" << endl;
	test_single3();
	
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
	cout<<"  testing unordered_map<Motif, pair<int, double>> (existing overloading function)" << endl;
	test_ummp();

	cout << "  testing to string" << endl;
	test_tostring();

	cout << "  testing from string" << endl;
	test_fromstring();
}
