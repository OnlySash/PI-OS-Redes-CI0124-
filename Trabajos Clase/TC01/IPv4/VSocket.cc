/**
  *  Universidad de Costa Rica
  *  ECCI
  *  CI0123 Proyecto integrador de redes y sistemas operativos
  *  2026-ii
  *  Grupos: 2 y 5
  *
  *******   VSocket base class implementation
  *
  * (Fedora version)
  *
 **/

#include <sys/socket.h>
#include <arpa/inet.h>		// ntohs, htons
#include <stdexcept>            // runtime_error
#include <cstring>		// memset
#include <netdb.h>		// getaddrinfo, freeaddrinfo
#include <unistd.h>		// close
/*
#include <cstddef>
#include <cstdio>

//#include <sys/types.h>
*/
#include "VSocket.h"


/**
  *  Class creator (constructor)
  *     use Unix socket system call
  *
  *  @param     char t: socket type to define
  *     's' for stream
  *     'd' for datagram
  *  @param     bool ipv6: if we need a IPv6 socket
  *
 **/
void VSocket::Init( char t, bool IPv6 ){

   int sockType = SOCK_STREAM;
   this->type = t;

   if (IPv6) {
      this->sockId= socket(AF_INET6, sockType, 0);
   } else {
      this->sockId = socket(AF_INET, sockType, 0);
   }

   if( -1 == this->sockId ) {
      throw std::runtime_error( "VSocket::Init()" );
   }
}


/**
  * Class destructor
  *
 **/
VSocket::~VSocket() {
   this->Close();
}


/**
  * Close method
  *    use Unix close system call (once opened a socket is managed like a file in Unix)
  *
 **/
void VSocket::Close(){
   int st = close(this->sockId);

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::Close()" );
   }

}


/**
  * TryToConnect method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dot notation, example "10.84.166.62"
  * @param      int port: process address, example 80
  *
 **/
int VSocket::TryToConnect( const char * hostip, int port ) {

   int st;
   
   struct sockaddr_in  host4;
   memset( (char *) &host4, 0, sizeof( host4 ) );
   host4.sin_family = AF_INET;
   st = inet_pton( AF_INET, hostip, &host4.sin_addr );

   if ( -1 == st ) {
      throw std::runtime_error( "VSocket::TryToConnect" );
   }

   host4.sin_port = htons( port );
   st = connect( this->sockId, (sockaddr *) &host4, sizeof( host4 ) );
   if ( -1 == st ) {
      throw( std::runtime_error( "VSocket::TryToConnect, connect" ) );
   }
   
   return st;

}


/**
  * TryToConnect method
  *   use "connect" Unix system call
  *
  * @param      char * host: host address in dns notation, example "os.ecci.ucr.ac.cr"
  * @param      char * service: process address, example "http"
  *
 **/
int VSocket::TryToConnect( const char *host, const char *service ) {
   struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *rp;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = this->IPv6 ? AF_INET6 : AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    int st = getaddrinfo(
        host,
        service,
        &hints,
        &result
    );

    if (st != 0) {
        throw std::runtime_error(
            "VSocket::TryToConnect(): getaddrinfo()"
        );
    }

    /*
     * Try each address returned by getaddrinfo().
     */
    for (rp = result; rp != nullptr; rp = rp->ai_next) {

        st = connect(
            this->sockId,
            rp->ai_addr,
            rp->ai_addrlen
        );

        if (st == 0) {
            freeaddrinfo(result);
            return 0;
        }
    }

    freeaddrinfo(result);

    throw std::runtime_error(
        "VSocket::TryToConnect(): connect()"
    );
}
