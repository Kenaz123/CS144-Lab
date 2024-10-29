#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return bytes_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return consecutive_retransmissions_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  // Your code here.
  uint64_t window = max( window_size_, static_cast<uint64_t>( 1 ) );
  while ( bytes_in_flight_ < window ) {
    TCPSenderMessage seg;
    if ( !syn_sent_ ) {
      seg.SYN = true;
      syn_sent_ = true;
    }
    auto payload_size = min( TCPConfig::MAX_PAYLOAD_SIZE,
                             min( window - bytes_in_flight_ - static_cast<uint64_t>( seg.SYN ? 1 : 0 ),
                                  input_.reader().bytes_buffered() ) );
    string payload;
    while ( payload.size() < payload_size ) {
      auto view = input_.reader().peek();
      if ( view.empty() ) {
        throw std::runtime_error( "Reader::peek() returned empty string_view" );
      }
      view = view.substr( 0, payload_size - payload.size() );
      payload += view;
      input_.reader().pop( view.size() );
    }
    seg.payload = payload;
    if ( !fin_sent_ && input_.reader().is_finished() && bytes_in_flight_ + seg.sequence_length() < window ) {
      seg.FIN = true;
      fin_sent_ = true;
    }
    if ( seg.sequence_length() == 0 )
      break; // empty message

    seg.seqno = get_next_seqno();
    seg.RST = input_.has_error();
    transmit( seg );
    if ( !timer_running_ ) {
      timer_running_ = true;
      timer_elapsed_ = 0;
    }
    segments_outstanding_.push( seg );
    bytes_in_flight_ += seg.sequence_length();
    next_abs_seqno_ += seg.sequence_length();
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // Your code here.
  TCPSenderMessage msg;
  msg.seqno = isn_ + next_abs_seqno_;
  msg.RST = input_.has_error();
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  msg.RST ? input_.set_error() : void();
  if ( !msg.ackno.has_value() ) {
    window_size_ = msg.window_size;
    return;
  }
  auto abs_ackno = msg.ackno.value().unwrap( isn_, next_abs_seqno_ );
  if ( abs_ackno > next_abs_seqno_ )
    return;
  while ( !segments_outstanding_.empty() ) {
    auto& seg = segments_outstanding_.front();
    if ( seg.seqno.unwrap( isn_, next_abs_seqno_ ) + seg.sequence_length() <= abs_ackno ) {
      bytes_in_flight_ -= seg.sequence_length();
      segments_outstanding_.pop();
      timer_elapsed_ = 0;
      consecutive_retransmissions_ = 0;
      rto_ = initial_RTO_ms_;
    } else {
      break;
    }
  }
  if ( bytes_in_flight_ == 0 ) {
    timer_running_ = false;
  }
  window_size_ = msg.window_size;
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  if ( !timer_running_ ) {
    return;
  }
  timer_elapsed_ += ms_since_last_tick;
  if ( timer_elapsed_ >= rto_ ) {
    transmit( segments_outstanding_.front() ); 
    if ( window_size_ > 0 ) {
      consecutive_retransmissions_++;
      rto_ *= 2;
    }
    timer_elapsed_ = 0;
  }
}
