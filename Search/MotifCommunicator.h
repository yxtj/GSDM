#pragma once
#include "Motif.h"

class MotifCommunicator
{
	int rank, size;
	//static const int TAG_SHUFFLE = 0;
	//static const int TAG_SHUFFLE_END = 1;
	static const int TAG_DATA = 0;
	static const int TAG_END = 1;
public:
	MotifCommunicator();

	std::vector<Motif> shuffle(std::vector<Motif>& motifs);
	std::vector<Motif> gather(std::vector<Motif>& motifs, const int target = 0);

private:
	int hasher(const Motif& m);

	std::vector<Motif> _shuffle_receive_thread();
};

class MotifSender {
	const int target, tag;
	const size_t bufSize;
	size_t restSize;
	size_t numMotif;
	char* buffer;
	char* p;
public:
	MotifSender(const int target, const int tag, const size_t bufSize);
	~MotifSender();
	void send(const Motif& m);
	void flush();
public:
	size_t estimateSize(const Motif& m) const;
};