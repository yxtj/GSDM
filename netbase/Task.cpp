#include "Task.h"
//#include <google/protobuf/message_lite.h>

//Task::Task(int s_d,int type,const google::protobuf::MessageLite& msg,const MsgHeader& h):
//		Task(s_d,type)
//{
////	const char* p=reinterpret_cast<const char*>(&h);
////	payload.append(p, p+sizeof(h));
//	msg.AppendToString(&payload);
//}
//
//void Task::Decode(google::protobuf::MessageLite& msg, const std::string& data){
////	msg.ParseFromArray(data.data() + sizeof(MsgHeader), data.size() - sizeof(MsgHeader));
//	msg.ParseFromArray(data.data(), data.size());
//}
