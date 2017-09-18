
#ifndef ServerSocket_class
#define ServerSocket_class

#include "Socket.h"

class ServerSocket : protected Socket
{
 public:
  ServerSocket ( int port );
  ServerSocket (){};
  virtual ~ServerSocket();
  const ServerSocket& operator << ( const std::string& ) const;
  const ServerSocket& operator >> ( std::string& ) const;
  void accept ( ServerSocket& );

  void cleanCon(ServerSocket& s){
	  cleanConnection(s.m_sock);
  }
};


#endif
