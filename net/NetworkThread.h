#pragma once
#include <thread>
#include <mutex>
#include <functional>
#include <string>
#include <deque>
#include <vector>
#include "../netbase/Task.h"
#include "../netbase/RPCInfo.h"
#include "../serialization/serialization.h"

//typedef google::protobuf::MessageLite MessageLite;

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
	void readAny(std::string& data, int *sourcsrcRete=nullptr, int *typeRet=nullptr);
	// Unblocked read for the given source and message type.
	bool tryReadAny(std::string& data, int *sosrcReturce=nullptr, int *typeRet=nullptr);

	template <class T>
	int send(int dst, int tag, const T& msg) {
		std::string s = serialize(msg);
		return send(new Task(dst, tag, move(s)));
	}
	// Enqueue the given request to pending buffer for transmission.
	//int send(int dst, int tag, const MessageLite &msg);
	// Directly send the request bypassing the pending buffer.
	//int sendDirect(int dst, int method, const MessageLite &msg);

	template <class T>
	void broadcast(int tag, const T& msg) {
		std::string s = serialize(msg);
		return broadcast(new Task(Task::ANY_DST, tag, move(s)));
	}
	//void broadcast(int method, const MessageLite& msg);

	void flush();
	void shutdown();

	int id() const;
	int size() const;

	static NetworkThread *GetInstance();
	static void Init(int argc, char* argv[]);

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
	int send(Task *req);
	// Directly (Physically) send the request.
	int sendDirect(Task *req);
	int broadcast(Task *req);


	bool checkReceiveQueue(std::string& data, TaskBase& info);

	void Run();

	static NetworkThread* self;
	NetworkThread();
};

