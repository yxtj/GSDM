// UnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "declare.h"

using namespace std;

int main(int argc, char* argv[])
{
	if(0){
		cout << "Test module net" << endl;
		testNet(argc, argv);
	}
	if(1) {
		cout << "Test serailization" << endl;
		testSerialzation(argc, argv);
	}
	
	return 0;
}

