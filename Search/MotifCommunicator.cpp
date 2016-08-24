#include "stdafx.h"
#include "MotifCommunicator.h"
#include "serialization.h"

using namespace std;

const int MotifCommunicator::TAG_END = 0;
const int MotifCommunicator::TAG_DATA = 1;

MotifCommunicator::MotifCommunicator()
{
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
}

bool MotifCommunicator::read(Motif & res)
{
	return false;
}

void MotifCommunicator::send(const Motif & m)
{
	send(m, hasher(m));
}

void MotifCommunicator::send(const Motif & m, const size_t target)
{
}

void MotifCommunicator::terminate()
{
	for(int i = 0; i < size; ++i) {
		if(i == rank)
			continue;

	}
}

std::vector<Motif> MotifCommunicator::shuffle(std::vector<Motif>& motifs)
{
	if(size == 1)
		return motifs;
	vector<Motif> res;
	//receive
	packaged_task<vector<Motif>()> taskRec(bind(&MotifCommunicator::_shuffle_receive_thread, this));
	future<vector<Motif>> resRec = taskRec.get_future();
	thread tRec(move(taskRec));
	//send:
	allocator<MotifSender> alc;
	MotifSender* senders = alc.allocate(size);
	const int bufSize = 32 * 1024;
	for(int i = 0; i < size;++i) {
		if(i != rank)
			alc.construct(senders + i, i, TAG_DATA, TAG_END, bufSize);
	}
	for(Motif& m : motifs) {
		int tgt = hasher(m);
		if(tgt == rank)
			res.push_back(move(m));
		else {
			senders[tgt].send(m);
		}
	}
	//flush
	for(int i = 0; i < size; ++i) {
		if(i != rank) {
			senders[i].flush();
			alc.destroy(senders + i);
		}
	}
	alc.deallocate(senders, size);

	tRec.join();
	vector<Motif> tmp = resRec.get();
	if(res.size() < tmp.size())
		swap(res, tmp);
	move(tmp.begin(), tmp.end(), back_inserter(res));
	
	return res;
}

std::vector<Motif> MotifCommunicator::gather(std::vector<Motif>& motifs, const int target)
{
	if(size == 1)
		return motifs;
	//TODO: not implemented
	return std::vector<Motif>();
}

int MotifCommunicator::hasher(const Motif & m)
{
	return hash<Motif>()(m) % size;
}

std::vector<Motif> MotifCommunicator::_shuffle_receive_thread()
{
	vector<Motif> res;
	bool finish = false;
	const int bufSize = 32 * 1024;
	char *buf = new char[bufSize];
	do {
		MPI_Status st;
		MPI_Recv(buf, bufSize, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
		if(st.MPI_TAG == TAG_END) {
			finish = true;
		} else {
			auto tmp = deserializeVM(buf);
			move(tmp.begin(), tmp.end(), back_inserter(res));
		}
	} while(!finish);
	delete[] buf;
	return res;
}

// MotifReceiver:

MotifReceiver::MotifReceiver(const int target, const int tag, const int termTag, const size_t bufSize)
	: target(target), tag(tag), termTag(termTag), bufSize(bufSize),
	restSize(bufSize), numMotif(0), buffer(new char[bufSize]), p(buffer)
{
}

MotifReceiver::~MotifReceiver()
{
	delete buffer;
}

bool MotifReceiver::receive(std::vector<Motif> & res)
{
	MPI_Status st;
	MPI_Recv(buffer, bufSize, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
	if(st.MPI_TAG == termTag) {
		return false;
	} else {
		auto tmp = deserializeVM(buffer);
		move(tmp.begin(), tmp.end(), back_inserter(res));
	}
	return true;
}

// MotifSender:

MotifSender::MotifSender(const int target, const int tag, const int termTag, const size_t bufSize)
	: target(target), tag(tag), termTag(termTag), bufSize(bufSize),
	restSize(bufSize), numMotif(0), buffer(new char[bufSize]), p(buffer)
{
	p += sizeof(int);
	restSize -= sizeof(int);
}

MotifSender::~MotifSender()
{
	delete[] buffer;
}

void MotifSender::send(const Motif & m)
{
	size_t s = estimateBufferSize(m);
	if(s > restSize) {
		flush();
	} else {
		p = serialize(p, m);
		restSize -= s;
		++numMotif;
	}
}

void MotifSender::flush()
{
	if(numMotif == 0)
		return;
	*reinterpret_cast<int*>(buffer) = numMotif;
	MPI_Send(buffer, p - buffer, MPI_CHAR, target, tag, MPI_COMM_WORLD);
	numMotif = 0;
	p = buffer + sizeof(int);
	restSize = sizeof(int);
}

void MotifSender::terminate()
{
	flush();
	MPI_Send(buffer, 1, MPI_CHAR, target, termTag, MPI_COMM_WORLD);
}

