#pragma once
#include <thread>
#include <mutex>
#include <functional>
#include <string>
#include <deque>
#include <vector>
#include "../netbase/Task.h"
#include "../serialization/serialization.h"


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

	// Send the message via a pending buffer for transmission.
	int send(int dst, int tag, std::string&& msg) {
		return send(new Task(dst, tag, move(msg)));
	}
	template <class T>
	int send(int dst, int tag, const T& msg) {
		std::string s = serialize(msg);
		return send(new Task(dst, tag, move(s)));
	}
	// Directly send the message bypassing the pending buffer.
	int sendDirect(int dst, int tag, std::string&& msg) {
		return sendDirect(new Task(dst, tag, move(msg)));
	}
	template <class T>
	int sendDirect(int dst, int tag, const T& msg) {
		std::string s = serialize(msg);
		return sendDirect(new Task(dst, tag, move(s)));
	}
	// Broadcast message to all OTHERS via a pending buffer
	int broadcast(int tag, std::string&& msg) {
		return broadcast(new Task(Task::ANY_DST, tag, move(msg)));
	}
	template <class T>
	int broadcast(int tag, const T& msg) {
		std::string s = serialize(msg);
		return broadcast(new Task(Task::ANY_DST, tag, move(s)));
	}

	void flush();

	int id() const;
	int size() const;

	static NetworkThread *GetInstance();
	static void Init(int argc, char* argv[]);
	// finish current tasks and terminate
	static void Shutdown();
	// abandon ongoing tasks and terminate all network related functions
	static void Terminate();

	bool pause_=false;

	uint64_t stat_send_pkg, stat_recv_pkg;
	uint64_t stat_send_byte, stat_recv_byte;

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

