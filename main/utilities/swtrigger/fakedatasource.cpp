#include "fakedatasource.h"





std::list<zmq::message_t*> getMessage(zmq::socket_t* pSock)
{
  std::list<zmq::message_t*> result;
  do {
    zmq::message_t* item = new zmq::message_t;
    pSock->recv(item);
    result.push_back(item);
  } while (more(pSock));
  
  return result;
}
bool more(zmq::socket_t* pSock)
{
  int64_t flag(0);
  size_t  flagSize(sizeof(flag));
  
  pSock->getsockopt(ZMQ_RCVMORE, &flag, &flagSize);
  return (flag == 1);
}

void deleteMessage(std::list<zmq::message_t*> msg)
{
  while (!msg.empty()) {
    delete msg.front();
    msg.pop_front();
  }
}

