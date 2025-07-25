#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <functional>
#include <map>

class RetransmissionTimer
{
  uint64_t initial_rto_ms_;
  uint64_t current_rto_ms_;
  uint64_t time_elapsed_ms_ {};
  bool is_active_ { false }; // Whether the timer is currently active

public:
  explicit RetransmissionTimer( uint64_t initial_rto_ms )
    : initial_rto_ms_( initial_rto_ms ), current_rto_ms_( initial_rto_ms )
  {}

  void start()
  {
    is_active_ = true;
    time_elapsed_ms_ = 0;
  }

  void stop()
  {
    is_active_ = false;
    time_elapsed_ms_ = 0;
    current_rto_ms_ = initial_rto_ms_;
  }

  void reset() { current_rto_ms_ = initial_rto_ms_; }

  void double_current_tro() { current_rto_ms_ = 2 * current_rto_ms_; }

  bool is_expired() const { return is_active_ && time_elapsed_ms_ >= current_rto_ms_; }

  bool is_running() const { return is_active_; }

  void time_elapsed( uint64_t ms )
  {
    if ( is_active_ ) {
      time_elapsed_ms_ += ms;
    }
  }
};

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
  uint64_t sequence_numbers_in_flight() const;  // For testing: how many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // For testing: how many consecutive retransmissions have happened?
  const Writer& writer() const { return input_.writer(); }
  const Reader& reader() const { return input_.reader(); }
  Writer& writer() { return input_.writer(); }

private:
  Reader& reader() { return input_.reader(); }

  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

  RetransmissionTimer timer_ { initial_RTO_ms_ };
  uint64_t next_seqno_ {};            // The next sequence number to be sent
  uint64_t last_sent_seqno_ {};       // The last sequence number sent
  uint64_t last_ackno_ {};            // The last ACK number received, also the left edge of the sender's window
  uint64_t rwindow_ {};               // The right edge of the sender's window
  uint64_t sender_window_size_ = 1;   // The sender's window size
  uint64_t receiver_window_size_ = 1; // The receiver's window size
  uint64_t consecutive_retransmissions_ {}; // How many consecutive retransmissions have happened
  bool SYN {};                       // Whether the TCPSender has sent SYN flag
  bool FIN {};                       // Whether the TCPSender has sent FIN flag
  bool zero_windowsize_received_ {}; // Whether the TCPSender has received a zero window size from the receiver

  std::map<uint64_t, TCPSenderMessage> outstanding_segments_ {}; // Maps sequence number to the segment
};
