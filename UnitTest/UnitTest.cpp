// UnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "declare.h"

using namespace std;

int main(int argc, char* argv[])
{
	if(0){
		cout << "Test module net-protobuf" << endl;
		testNet(argc, argv);
	}
	if(0) {
		cout << "Test module serailization" << endl;
		testSerialzation(argc, argv);
	}
	if(0) {
		cout << "Test module net" << endl;
		testNet(argc, argv);
	}
	if(1) {
		cout << "Test module message driver" << endl;
		testMsgDriver(argc, argv);
	}
	
	return 0;
}

