#include "stdafx.h"
#include "Network.h"
#include "serialization.h"


const size_t Network::bufSize = 64 * 1024;

using namespace std;

Network::Network() :
	restBufSizeSend(bufSize), bufSend(new char[bufSize]), pSend(bufSend),
	restBufSizeRecv(bufSize), bufRecv(new char[bufSize]), pRecv(bufRecv)
{
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
}


Network::~Network()
{
	delete[] bufSend;
	delete[] bufRecv;
}

int Network::getSize() const
{
	return size;
}

int Network::getRank() const
{
	return rank;
}

void Network::sendMotif(const int target, const Motif & m)
{
	size_t s = estimateBufferSize(m);
	if(s > restBufSizeSend) {
		do {
			restBufSizeSend *= 2;
		} while(s > restBufSizeSend);
		delete[] bufSend;
		bufSend = new char[restBufSizeSend];
		pSend = bufSend;
	}
	pSend = serialize(bufSend, m);
//	cout << "net: send " << s << endl;
	MPI_Send(bufSend, s, MPI_CHAR, target, TAG_MOTIF, MPI_COMM_WORLD);
}

bool Network::readMotif(Motif & res, int & source)
{
	MPI_Status st;
	MPI_Recv(bufRecv, restBufSizeRecv, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
	int s;
	MPI_Get_count(&st, MPI_CHAR, &s);
//	cout << "net: recv " << s <<" tag: "<<st.MPI_TAG<< endl;
	if(st.MPI_TAG == TAG_END) {
		return false;
	}
	tie(res,ignore)= deserialize(bufRecv);
	source = st.MPI_SOURCE;
	return true;
}

void Network::sendCount(const int target, const std::pair<int, int>& c)
{
	int* p = reinterpret_cast<int*>(bufSend);
	*p++ = c.first;
	*p++ = c.second;
	int s = 2 * sizeof(int);
	MPI_Send(bufSend, s, MPI_CHAR, target, TAG_COUNT_PAIR, MPI_COMM_WORLD);
}

bool Network::readCount(std::pair<int, int>& res, int & source)
{
	MPI_Status st;
	MPI_Recv(bufRecv, restBufSizeRecv, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
	if(st.MPI_TAG == TAG_END) {
		return false;
	}
	res.first = *reinterpret_cast<int*>(bufRecv);
	res.second= *reinterpret_cast<int*>(bufRecv+sizeof(int));
	source = st.MPI_SOURCE;
	return true;
}

void Network::sendCountPos(const int target, const int c)
{
	*reinterpret_cast<int*>(bufSend) = c;
	MPI_Send(bufSend, 1, MPI_INT, target, TAG_COUNT_POS, MPI_COMM_WORLD);
}

bool Network::readCountPos(int & res, int & source)
{
	MPI_Status st;
	MPI_Recv(bufRecv, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
	if(st.MPI_TAG == TAG_END) {
		return false;
	}
	res= *reinterpret_cast<int*>(bufRecv);
	return true;
}

void Network::sendCountNeg(const int target, const int c)
{
	*reinterpret_cast<int*>(bufSend) = c;
	MPI_Send(bufSend, 1, MPI_INT, target, TAG_COUNT_NEG, MPI_COMM_WORLD);
}

bool Network::readCountNeg(int & res, int & source)
{
	MPI_Status st;
	MPI_Recv(bufRecv, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &st);
	if(st.MPI_TAG == TAG_END) {
		return false;
	}
	res = *reinterpret_cast<int*>(bufRecv);
	return true;
}

void Network::sendEnd(const int target)
{
	MPI_Send(bufSend, 1, MPI_CHAR, target, TAG_END, MPI_COMM_WORLD);
}

void Network::sendEnd()
{
	for(int i = 0; i < size; ++i) {
		if(i != rank)
			continue;
		sendEnd(i);
	}
}

