#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>

class TCPSender
{
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ), isn_( isn ), initial_RTO_ms_( initial_RTO_ms )
  {}

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
  uint64_t get_next_abs_seqno_() const { return next_abs_seqno_; };
  Wrap32 get_next_seqno() const { return isn_ + next_abs_seqno_; };

  Writer& writer() { return input_.writer(); }
  const Writer& writer() const { return input_.writer(); }

  // Access input stream reader, but const-only (can't read from outside)
  const Reader& reader() const { return input_.reader(); }

private:
  // Variables initialized in constructor
  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  uint64_t next_abs_seqno_ { 0 };  // Next absolute sequence number to send
  uint64_t window_size_ { 1 };     // Current window size
  uint64_t bytes_in_flight_ { 0 }; // Number of bytes in flight
  // std::queue<TCPSenderMessage> send_queue_ {}; // Queue of messages to send
  bool syn_sent_ { false };                              // Has the SYN flag been sent?
  bool fin_sent_ { false };                              // Has the FIN flag been sent?
  uint64_t consecutive_retransmissions_ { 0 };           // Number of consecutive retransmissions
  uint64_t rto_ { initial_RTO_ms_ };                     // Retransmission Timeout
  uint64_t timer_elapsed_ { 0 };                         // Time elapsed since last tick
  bool timer_running_ { false };                         // Is the timer running?
  std::queue<TCPSenderMessage> segments_outstanding_ {}; // Outstanding segments
};
