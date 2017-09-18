// Implementation of the Socket class.


#include "Socket.h"
#include "string.h"
#include <cstdlib>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <iostream>
using namespace std;

Socket::Socket() :
  m_sock ( -1 )
{

  memset ( &m_addr,
	   0,
	   sizeof ( m_addr ) );

}

Socket::~Socket()
{
  if ( is_valid() )
    ::close ( m_sock );
}

bool Socket::create()
{
  m_sock = socket ( AF_INET,
		    SOCK_STREAM,
		    0 );

  if ( ! is_valid() )
    return false;


  // TIME_WAIT - argh
    /*************************************************************/
    /* Allow socket descriptor to be reuseable                   */
    /*************************************************************/
  int on = 1;
  if ( setsockopt ( m_sock, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
    return false;

  set_non_blocking(true);

  return true;

}



bool Socket::bind ( const int port )
{

  if ( ! is_valid() )
    {
      return false;
    }



  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;
  m_addr.sin_port = htons ( port );

  int bind_return = ::bind ( m_sock,
			     ( struct sockaddr * ) &m_addr,
			     sizeof ( m_addr ) );


  if ( bind_return == -1 )
    {
      return false;
    }

  return true;
}


bool Socket::listen()
{
  if ( ! is_valid() )
    {
      return false;
    }

  int listen_return = ::listen ( m_sock, MAXCONNECTIONS );


  if ( listen_return == -1 )
    {
      return false;
    }

  /*************************************************************/
     /* Initialize the master fd_set                              */
     /*************************************************************/
  FD_ZERO(&master_set);
  max_sd = m_sock;
  FD_SET(m_sock, &master_set);
  return true;
}


bool Socket::accept ( Socket& new_socket )
{

   int addr_length = sizeof ( m_addr );
//  new_socket.m_sock = ::accept ( m_sock, ( sockaddr * ) &m_addr, ( socklen_t * ) &addr_length );
//
//  if ( new_socket.m_sock <= 0 )
//    return false;
//  else
//    return true;

  fd_set working_set;
  memcpy(&working_set, static_cast<const void*>(&master_set), sizeof(master_set));

  int rc = select(max_sd + 1, &working_set, NULL, NULL, NULL);
  /**********************************************************/
  /* Check to see if the select call failed.                */
  /**********************************************************/
  if (rc < 0)
  {
     perror("  select() failed");
     return false;
  }


  /**********************************************************/
  /* One or more descriptors are readable.  Need to         */
  /* determine which ones they are.                         */
  /**********************************************************/
  int desc_ready = rc;
  for (int i=0; i <= max_sd  &&  desc_ready > 0; ++i)
  {
     /*******************************************************/
     /* Check to see if this descriptor is ready            */
     /*******************************************************/
     if (FD_ISSET(i, &working_set))
     {
        /****************************************************/
        /* A descriptor was found that was readable - one   */
        /* less has to be looked for.  This is being done   */
        /* so that we can stop looking at the working set   */
        /* once we have found all of the descriptors that   */
        /* were ready.                                      */
        /****************************************************/
        desc_ready -= 1;

        /****************************************************/
        /* Check to see if this is the listening socket     */
        /****************************************************/
        if (i == m_sock)
        {
//           printf("  Listening socket is readable\n");
           /*************************************************/
           /* Accept all incoming connections that are      */
           /* queued up on the listening socket before we   */
           /* loop back and call select again.              */
           /*************************************************/
           //do
           //{
              /**********************************************/
              /* Accept each incoming connection.  If       */
              /* accept fails with EWOULDBLOCK, then we     */
              /* have accepted all of them.  Any other      */
              /* failure on accept will cause us to end the */
              /* server.                                    */
              /**********************************************/
        	  new_socket.m_sock  =  ::accept( m_sock, ( sockaddr * ) &m_addr, ( socklen_t * ) &addr_length );
              if (new_socket.m_sock  < 0)
              {
                 if (errno != EWOULDBLOCK)
                 {
                    perror("  accept() failed");
                 }
                 return false;
              }

              /**********************************************/
              /* Add the new incoming connection to the     */
              /* master read set                            */
              /**********************************************/
              //printf("  New incoming connection - %d\n", new_sd);
              FD_SET(new_socket.m_sock , &master_set);
              if (new_socket.m_sock  > max_sd){
                 max_sd = new_socket.m_sock;
              }
              /**********************************************/
              /* Loop back up and accept another incoming   */
              /* connection                                 */
              /**********************************************/
         //  } while (new_sd != -1);
           return true;
        }
     }
  }
        return true;

}


bool Socket::send ( const std::string s ) const
{
  int status = ::send ( m_sock, s.c_str(), s.size(), MSG_NOSIGNAL );
  if ( status == -1 )
    {
      return false;
    }
  else
    {
      return true;
    }
}


int Socket::recv ( std::string& s ) const
{
  char buf [ MAXRECV + 1 ];

  s = "";

  memset ( buf, 0, MAXRECV + 1 );

  int status = ::recv ( m_sock, buf, MAXRECV, 0 );

  if ( status == -1 )
    {
      std::cout << "status == -1   errno == " << errno << "  in Socket::recv\n";
      return 0;
    }
  else if ( status == 0 )
    {
      return 0;
    }
  else
    {
      s = buf;
      return status;
    }
}



bool Socket::connect ( const std::string host, const int port )
{
  if ( ! is_valid() ) return false;

  m_addr.sin_family = AF_INET;
  m_addr.sin_port = htons ( port );

  int status = inet_pton ( AF_INET, host.c_str(), &m_addr.sin_addr );

  if ( errno == EAFNOSUPPORT ) return false;

  status = ::connect ( m_sock, ( sockaddr * ) &m_addr, sizeof ( m_addr ) );

  if ( status == 0 )
    return true;
  else
    return false;
}

void Socket::set_non_blocking ( const bool b )
{

  int opts;

  opts = fcntl ( m_sock,
		 F_GETFL );

  if ( opts < 0 )
    {
      return;
    }

  if ( b )
    opts = ( opts | O_NONBLOCK );
  else
    opts = ( opts & ~O_NONBLOCK );

  fcntl ( m_sock,
	  F_SETFL,opts );

}
