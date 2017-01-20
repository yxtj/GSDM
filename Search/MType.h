#pragma once

struct MType {
	// Basic control
	static constexpr int CReply = 0;
	static constexpr int CReg = 1;
	static constexpr int CReady = 2;
	static constexpr int CEnd = 3;

	// Candidate edge related
	static constexpr int CEInit = 10;
	static constexpr int CEUpdateThreshold = 11;
	static constexpr int CERemove = 12;

	// Motif
	static constexpr int MGenerated = 20;
	static constexpr int MFailed = 21;
	static constexpr int MGather = 22;
	
	// Staticstics
	static constexpr int SGather = 40;
};
