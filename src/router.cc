#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  // Your code here.
  _routes.push_back( RouteEntry( route_prefix, prefix_length, next_hop, interface_num ) );
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  // Your code here.
  for ( auto& interface : _interfaces ) {
    auto& queue = interface->datagrams_received();
    while ( !queue.empty() ) {
      auto dgram = queue.front();
      if ( dgram.header.ttl <= 1 )
        return;
      // Longest prefix match
      uint32_t dest_ip = dgram.header.dst;
      int match_id = -1;
      int match_len = -1;
      for ( size_t i = 0; i < _routes.size(); i++ ) {
        auto route = _routes[i];
        uint32_t route_prefix = route.route_prefix;
        uint8_t prefix_length = route.prefix_length;
        uint32_t mask = ( prefix_length == 0 ) ? 0 : numeric_limits<int>::min() >> ( prefix_length - 1 );
        if ( ( dest_ip & mask ) == route_prefix ) {
          if ( prefix_length > match_len ) {
            match_len = prefix_length;
            match_id = i;
          }
        }
      }
      if ( match_id == -1 )
        return;
      dgram.header.ttl--;
      dgram.header.compute_checksum();
      auto next_hop = _routes[match_id].next_hop;
      auto interface_num = _routes[match_id].interface_num;
      if ( next_hop.has_value() ) {
        _interfaces[interface_num]->send_datagram( dgram, next_hop.value() );
      } else {
        _interfaces[interface_num]->send_datagram( dgram, Address::from_ipv4_numeric( dest_ip ) );
      }
      queue.pop();
      // Your code here.
    }
  }
}
