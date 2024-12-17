#include <iostream>

#include "arp_message.hh"
#include "exception.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  // Your code here.
  EthernetFrame ethernetframe;
  ethernetframe.header.src = ethernet_address_;
  if ( !arp_table_.contains( next_hop.ipv4_numeric() ) ) {
    if ( arp_life_.contains( next_hop.ipv4_numeric() ) ) {
      return; // 发送还在lifetime内的ARP请求
    }
    ethernetframe.header.type = EthernetHeader::TYPE_ARP;
    ethernetframe.header.dst = ETHERNET_BROADCAST;
    ARPMessage arpmessage;
    arpmessage.opcode = ARPMessage::OPCODE_REQUEST;
    arpmessage.sender_ethernet_address = ethernet_address_;
    arpmessage.sender_ip_address = ip_address_.ipv4_numeric();
    arpmessage.target_ip_address = next_hop.ipv4_numeric();
    ethernetframe.payload = serialize( arpmessage );
    arp_life_.emplace( arpmessage.target_ip_address, make_pair( cur_time, ethernetframe ) );
    EthernetFrame waitframe{
        .header = {
          .dst = ETHERNET_BROADCAST,
          .src = ethernet_address_,
          .type = EthernetHeader::TYPE_IPv4,
        },
        .payload = serialize(dgram),
      };
    wait_queue_.emplace( next_hop.ipv4_numeric(), waitframe );
    // datagrams_received_.emplace(dgram);
    transmit( ethernetframe );
    return;
  }
  ethernetframe.header.type = EthernetHeader::TYPE_IPv4;
  ethernetframe.header.dst = arp_table_[next_hop.ipv4_numeric()].second;
  ethernetframe.payload = serialize( dgram );
  transmit( ethernetframe );
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  // Your code here.
  if ( frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST ) {
    return;
  }
  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram dgram;
    if ( parse( dgram, frame.payload ) ) {
      datagrams_received_.emplace( dgram );
      return;
    }
  }
  ARPMessage arpmessage;
  if ( parse( arpmessage, frame.payload ) ) {
    if ( arpmessage.target_ip_address != ip_address_.ipv4_numeric() ) {
      return;
    }
    EthernetFrame ethernetframe;
    arp_table_.emplace( arpmessage.sender_ip_address, make_pair( cur_time, arpmessage.sender_ethernet_address ) );
    if ( wait_queue_.contains( arpmessage.sender_ip_address ) ) {
      auto range = wait_queue_.equal_range( arpmessage.sender_ip_address );
      for ( auto it = range.first; it != range.second; it++ ) {
        it->second.header.dst = arpmessage.sender_ethernet_address;
        transmit( it->second );
      }
      wait_queue_.erase( range.first, range.second );
    }
    if ( arpmessage.opcode == ARPMessage::OPCODE_REQUEST ) {
      // send reply
      ethernetframe.header.src = ethernet_address_;
      ethernetframe.header.dst = arpmessage.sender_ethernet_address;
      ethernetframe.header.type = EthernetHeader::TYPE_ARP;
      ARPMessage arpmessage_reply;
      arpmessage_reply.opcode = ARPMessage::OPCODE_REPLY;
      arpmessage_reply.sender_ethernet_address = ethernet_address_;
      arpmessage_reply.sender_ip_address = ip_address_.ipv4_numeric();
      arpmessage_reply.target_ethernet_address = arpmessage.sender_ethernet_address;
      arpmessage_reply.target_ip_address = arpmessage.sender_ip_address;
      ethernetframe.payload = serialize( arpmessage_reply );
      transmit( ethernetframe );
      return;
    } else if ( arpmessage.opcode == ARPMessage::OPCODE_REPLY ) {
      // ethernetframe.header.src = ethernet_address_;
      // ethernetframe.header.dst = arpmessage.sender_ethernet_address;
      // ethernetframe.header.type = EthernetHeader::TYPE_IPv4;
      // ethernetframe.payload = serialize(datagrams_received_.front());
      // datagrams_received_.pop();
      if ( arp_life_.contains( arpmessage.sender_ip_address ) ) {
        arp_life_.erase( arpmessage.sender_ip_address );
      }
      // transmit(ethernetframe);
      return;
    }
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  cur_time += ms_since_last_tick;
  for ( auto& arp : arp_life_ ) {
    if ( cur_time - arp.second.first >= 5000 ) {
      arp.second.first = cur_time;
      transmit( arp.second.second );
    }
  }
  for ( auto it = arp_table_.begin(); it != arp_table_.end(); ) {
    if ( cur_time - it->second.first >= 30000 ) {
      it = arp_table_.erase( it );
    } else {
      it++;
    }
  }
}
