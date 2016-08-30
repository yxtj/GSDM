#pragma once
#include <thread>
#include <mutex>
#include <functional>
#include <string>
#include <deque>
#include <vector>
//#include <google/protobuf/message.h>
//#include "NetworkImplMPI.h"
#include "Task.h"
#include "RPCInfo.h"

typedef google::protobuf::Message Message;

class NetworkImplMPI;

// Hackery to get around mpi's unhappiness with threads.  This thread
// simply polls MPI continuously for any kind of update and adds it to
// a local queue.
class NetworkThread{
public:
	bool active() const;
	size_t pending_pkgs() const;
	int64_t pending_bytes() const;
	size_t unpicked_pkgs() const{
		return receive_buffer.size();
	}
	int64_t unpicked_bytes() const;

	// Blocking read for the given source and message type.
	void ReadAny(std::string& data, int *sourcsrcRete=nullptr, int *typeRet=nullptr);
	bool TryReadAny(std::string& data, int *sosrcReturce=nullptr, int *typeRet=nullptr);

	// Enqueue the given request to pending buffer for transmission.
	int Send(int dst, int tag, const Message &msg);
	// Directly send the request bypassing the pending buffer.
	int DSend(int dst, int method, const Message &msg);

	void Broadcast(int method, const Message& msg);

	void Flush();
	void Shutdown();

	int id() const;
	int size() const;

	static NetworkThread *Get();
	static void Init();

	bool pause_=false;

//	static constexpr int ANY_SRC = TaskBase::ANY_SRC;
//	static constexpr int ANY_TAG = TaskBase::ANY_TYPE;

private:
	bool running;
	bool done;
	NetworkImplMPI* net;
	mutable std::thread t_;

	//buffer for request to be sent, double buffer design for performance
	std::vector<Task*> ps_buffer_[2];
	std::vector<Task*>* pending_sends_=&ps_buffer_[0];
	unsigned ps_idx_=0;
	mutable std::recursive_mutex ps_lock;

	std::deque<std::pair<std::string,TaskBase> > receive_buffer;
	mutable std::recursive_mutex rec_lock;

	// Enqueue the given request to pending buffer for transmission.
	int Send(Task *req);
	// Directly (Physically) send the request.
	int DSend(Task *req);

	bool checkReceiveQueue(std::string& data, TaskBase& info);

	void Run();

	NetworkThread();
};
