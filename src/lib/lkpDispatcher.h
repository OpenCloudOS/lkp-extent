#ifndef LKP_DISPATCHER_H
#define LKP_DISPATCHER_H

#include "muduo/base/noncopyable.h"
#include "muduo/net/Callbacks.h"

#include <google/protobuf/message.h>

#include <map>

#include <type_traits>

typedef std::shared_ptr<google::protobuf::Message> MessagePtr;

class Callback : muduo::noncopyable
{
 public:
  virtual ~Callback() = default;
  virtual void onMessage(const muduo::net::TcpConnectionPtr&,
                         const MessagePtr& message,
                         muduo::Timestamp) const = 0;
};

//每种message对应一个CallbackT，内部包含一个callback_
template <typename T>
class CallbackT : public Callback
{
  static_assert(std::is_base_of<google::protobuf::Message, T>::value,
                "T must be derived from gpb::Message.");
 public:
  typedef std::function<void (const muduo::net::TcpConnectionPtr&,
                                const std::shared_ptr<T>& message,
                                muduo::Timestamp)> ProtobufMessageTCallback;

  CallbackT(const ProtobufMessageTCallback& callback)
    : callback_(callback)
  {
  }

  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 const MessagePtr& message,
                 muduo::Timestamp receiveTime) const override
  {
    std::shared_ptr<T> concrete = muduo::down_pointer_cast<T>(message);
    assert(concrete != NULL);
    callback_(conn, concrete, receiveTime);
  }

 private:
  ProtobufMessageTCallback callback_;
};

class lkpDispatcher
{
 public:
  typedef std::function<void (const muduo::net::TcpConnectionPtr&,
                                const MessagePtr& message,
                                muduo::Timestamp)> ProtobufMessageCallback;

  explicit lkpDispatcher(const ProtobufMessageCallback& defaultCb)
    : defaultCallback_(defaultCb)
  {
  }

  //得到根据类型生成的message后调用
  void onProtobufMessage(const muduo::net::TcpConnectionPtr& conn,
                         const MessagePtr& message,
                         muduo::Timestamp receiveTime) const
  {
    CallbackMap::const_iterator it = callbacks_.find(message->GetDescriptor());//利用message内的descriptor从哈系表取出正确CallbackT
    if (it != callbacks_.end())
    {
      it->second->onMessage(conn, message, receiveTime);//CallbackT内的onMessage是该类型消息的回调函数
    }
    else
    {
      defaultCallback_(conn, message, receiveTime);
    }
  }

  //T例如lkpMessage::Command
  template<typename T>
  void registerMessageCallback(const typename CallbackT<T>::ProtobufMessageTCallback& callback)
  {
    std::shared_ptr<CallbackT<T> > pd(new CallbackT<T>(callback));
    callbacks_[T::descriptor()] = pd;//从lkpMessage::Command中取出descriptor
  }

 private:
  typedef std::map<const google::protobuf::Descriptor*, std::shared_ptr<Callback> > CallbackMap;

  CallbackMap callbacks_;
  ProtobufMessageCallback defaultCallback_;
};
#endif  // LKP_DISPATCHER_H

