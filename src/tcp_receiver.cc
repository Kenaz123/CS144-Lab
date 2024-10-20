#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  if ( message.RST ) {
    reassembler_.reader().set_error();
    return;
  }
  if ( !_isn.has_value() ) {
    if ( !message.SYN ) {
      return;
    }
    _isn = message.seqno;
  }
  uint64_t checkpoint = reassembler_.writer().bytes_pushed(); // +1?
  uint64_t abs_seqno = message.seqno.unwrap( _isn.value(), checkpoint );
  uint64_t stream_index = abs_seqno - 1 + message.SYN;
  reassembler_.insert( stream_index, message.payload, message.FIN );
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  std::optional<Wrap32> ack;
  if ( !_isn.has_value() ) {
    ack = nullopt;
  } else {
    uint64_t abs_seq = reassembler_.writer().bytes_pushed() + 1 + ( reassembler_.writer().is_closed() ? 1 : 0 );
    ack.emplace( Wrap32::wrap( abs_seq, _isn.value() ) );
  }
  uint16_t capa = min( UINT16_MAX, static_cast<int>( reassembler_.writer().available_capacity() ) );
  bool rst = false;
  if ( reassembler_.writer().has_error() )
    rst = true;
  return TCPReceiverMessage { ack, capa, rst };
}
