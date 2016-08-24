#pragma once
#include <utility>
#include "Motif.h"

class Network
{
	int size, rank; // MPI related variables
	static const size_t bufSize;
	size_t restBufSizeSend;
	char *bufSend, *pSend;
	size_t restBufSizeRecv;
	char *bufRecv, *pRecv;
public:
	Network();
	~Network();

	int getSize() const;
	int getRank() const;

	void sendMotif(const int target, const Motif& m);
	bool readMotif(Motif& res, int& source);

	void sendCount(const int target, const std::pair<int, int>& c);
	bool readCount(std::pair<int, int>& res, int& source);
	void sendCountPos(const int target, const int c);
	bool readCountPos(int& res, int& source);
	void sendCountNeg(const int target, const int c);
	bool readCountNeg(int& res, int& source);

	void sendEnd(const int target);
	void sendEnd();

private:

public:
	static constexpr int TAG_END = 0;
	static constexpr int TAG_MOTIF = 1;
	static constexpr int TAG_COUNT_PAIR = 2;
	static constexpr int TAG_COUNT_POS = 2;
	static constexpr int TAG_COUNT_NEG = 2;

};

