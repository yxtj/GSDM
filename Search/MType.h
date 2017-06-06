#pragma once

struct MType {
	// Basic control
	static constexpr int CReply = 0;
	static constexpr int CReg = 1;
	static constexpr int CReady = 2;
	static constexpr int CEnd = 3;

	// Candidate edge related
	static constexpr int CEInit = 10;
	static constexpr int CEUsed = 12;
	static constexpr int CERemove = 13;
	static constexpr int CEUsage = 14;

	// Motif
	static constexpr int MNormal= 20;
	static constexpr int MAbondan = 21;
	static constexpr int MGather = 22;

	// Global progress flow
	static constexpr int GLevelFinish= 30;
	static constexpr int GSearchFinish= 31;
	static constexpr int GLowerBound = 32;
	static constexpr int GGatherLocalTopK = 33; // gather local top-k scores

	// Signature
	static constexpr int SGInit = 50;
	
	// Staticstics
	static constexpr int STGather = 60;
};
