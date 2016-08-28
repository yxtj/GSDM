#pragma once
#include "../common/Motif.h"

class MotifCommunicator
{
	int rank, size;
	//static const int TAG_SHUFFLE = 0;
	//static const int TAG_SHUFFLE_END = 1;
	static const int TAG_END;// = 0;
	static const int TAG_DATA;// = 1;
public:
	MotifCommunicator();

	// low level one step API
	bool read(Motif& res);
	void send(const Motif& m);
	void send(const Motif& m, const size_t target);
	void terminate();

	// high level API
	std::vector<Motif> shuffle(std::vector<Motif>& motifs);
	std::vector<Motif> gather(std::vector<Motif>& motifs, const int target = 0);

private:
	int hasher(const Motif& m);

	std::vector<Motif> _shuffle_receive_thread();
};

class MotifReceiver {
	const int target, tag, termTag;
	const size_t bufSize;
	size_t restSize;
	size_t numMotif;
	char* buffer;
	char* p;
public:
	MotifReceiver(const int target, const int tag, const int termTag, const size_t bufSize);
	~MotifReceiver();
	bool receive(std::vector<Motif>& res);
};

class MotifSender {
	const int target, tag, termTag;
	const size_t bufSize;
	size_t restSize;
	size_t numMotif;
	char* buffer;
	char* p;
public:
	MotifSender(const int target, const int tag, const int termTag, const size_t bufSize);
	~MotifSender();
	void send(const Motif& m);
	void flush();
	void terminate();
};