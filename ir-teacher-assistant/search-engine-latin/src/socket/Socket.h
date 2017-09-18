// Definition of the Socket class

#ifndef Socket_class
#define Socket_class


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>


const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 5;
const int MAXRECV = 8192;
//http://publib.boulder.ibm.com/infocenter/iseries/v5r3/index.jsp?topic=%2Frzab6%2Frzab6xnonblock.htm
class Socket
{
 public:
  Socket();
  virtual ~Socket();

  // Server initialization
  bool create();
  bool bind ( const int port );
  bool listen();
  bool accept ( Socket& );

  // Client initialization
  bool connect ( const std::string host, const int port );

  // Data Transimission
  bool send ( const std::string ) const;
  int recv ( std::string& ) const;


  void set_non_blocking ( const bool );

  bool is_valid() const { return m_sock != -1; }
  void cleanConnection(int con){
	  FD_CLR(con, &master_set);
	  if (con == max_sd)
	  {
		 while (FD_ISSET(max_sd, &master_set) == false)
			max_sd -= 1;
	  }
  }

 protected:
  	  int m_sock;
 private:


  sockaddr_in m_addr;
  fd_set master_set;
  int max_sd;

};


#endif
