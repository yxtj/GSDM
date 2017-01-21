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

	// Motif
	static constexpr int MNormal= 20;
	static constexpr int MAbondan = 21;
	static constexpr int MGather = 22;

	// Motif Level progress
	static constexpr int MLComplete = 23;

	// Top-k related
	static constexpr int TUpdateLB = 30;
	
	// Staticstics
	static constexpr int SGather = 40;
};
