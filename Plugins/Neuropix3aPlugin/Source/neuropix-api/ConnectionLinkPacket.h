#ifndef ConnectionLinkPacket_h_
#define ConnectionLinkPacket_h_

#include <deque>
#include <iosfwd>

class ConnectionLinkPacket : public std::deque<unsigned char>
{
public:
  ConnectionLinkPacket(){}
  ConnectionLinkPacket(char *, size_t size);
  ~ConnectionLinkPacket(){}
  // new[] allocated copy of packet. To be deleted with delete[]
  char * stream() const;

  void push_front(unsigned char v);
  void push_front(char v);
  void push_front(unsigned int v);

  void pack(unsigned int v, unsigned int nrBytes);
  unsigned int unpack(unsigned int nrBytes);
};

typedef std::deque<ConnectionLinkPacket> ConnectionLinkPacketList;


ConnectionLinkPacket & operator<<(ConnectionLinkPacket & p, unsigned char v);
ConnectionLinkPacket & operator<<(ConnectionLinkPacket & p, char v);
ConnectionLinkPacket & operator<<(ConnectionLinkPacket & p, unsigned short v);
ConnectionLinkPacket & operator<<(ConnectionLinkPacket & p, unsigned int v);
ConnectionLinkPacket & operator<<(ConnectionLinkPacket & p, const ConnectionLinkPacket & v);
ConnectionLinkPacket & operator>>(ConnectionLinkPacket & p, unsigned char & v);
ConnectionLinkPacket & operator>>(ConnectionLinkPacket & p, unsigned short & v);
ConnectionLinkPacket & operator>>(ConnectionLinkPacket & p, unsigned int & v);
ConnectionLinkPacket & operator>>(ConnectionLinkPacket & p, ConnectionLinkPacket & v);

std::ostream & operator<<(std::ostream & str, const ConnectionLinkPacket & v);
std::istream & operator>>(std::istream & str, ConnectionLinkPacket & p);
std::istream & operator>>(std::istream & str, ConnectionLinkPacketList & p);

#endif
