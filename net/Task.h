#pragma once
/*
 * Task.h
 *
 *  Created on: Nov 29, 2015
 *      Author: tzhou
 */
#include <string>
//#include <memory>

//namespace google{
//	namespace protobuf{
//		class MessageLite;
//	}
//}

struct MsgHeader{
	MsgHeader(const bool reply=false):is_reply(reply){}
	bool is_reply;
};

struct TaskBase{
	int src_dst;
	int type;
	static constexpr int ANY_SRC=-1;
	static constexpr int ANY_DST=-1;
	static constexpr int ANY_TYPE=-1;
};

struct TaskHeader :public TaskBase {
	int nBytes;
};

struct Task : public TaskBase{
	std::string payload;
	Task(int s_d,int type):TaskBase{s_d,type} {}//src_dst(s_d),type(type){}
	Task(int s_d,int type,std::string&& s):TaskBase{s_d,type},payload(s){}
	Task(int s_d,int type,const std::string& s):TaskBase{s_d,type},payload(s){}

//	Task(int s_d,int type,const google::protobuf::MessageLite& msg,const MsgHeader& h=MsgHeader(false));

//	static void Decode(google::protobuf::MessageLite& msg, const std::string& data);
//	void decode(google::protobuf::MessageLite& msg);
};

//inline void Task::decode(google::protobuf::MessageLite& msg){
//	Task::Decode(msg,payload);
//

//namespace std{
//template<>
//struct hash<dsm::Task>{
//	typedef dsm::Task argument_type;
//	typedef std::size_t result_type;
//	result_type operator()(const argument_type& t) const{
//		return std::hash<int>()(t.src_dst)^(std::hash<int>()(t.type)<<2);//^(std::hash(t.payload)<<4);
//	}
//};
//}
