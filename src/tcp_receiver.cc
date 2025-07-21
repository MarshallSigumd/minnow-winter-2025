#include "tcp_receiver.hh"
#include "debug.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // // Your code here.
  // debug( "unimplemented receive() called" );
  // (void)message;

  if ( message.RST ) {
    reassembler_.output_writer().close();
    return;
  }

  if ( message.FIN ) {
    FIN = true;
  }

  if ( message.SYN ) {
    zero_point_ = message.seqno;
    reassembler_.SYN = true;
    reassembler_.FIN = message.FIN;
    FIN = message.FIN;
  }

  uint64_t first_index = message.seqno.unwrap( zero_point_, reassembler_.next_pushed_index() ) + message.SYN;
  reassembler_.insert( first_index, message.payload, message.FIN );

  if ( FIN && reassembler_.writer().is_closed() ) {
    reassembler_.FIN = true;
  }

  // debug( "receive() called with seqno: {}, first_index: {}, payload size: {}, FIN: {}",
  //        message.seqno.raw_value_, first_index, message.payload.size(), message.FIN );
}

TCPReceiverMessage TCPReceiver::send() const
{
  // // Your code here.
  // debug( "unimplemented send() called" );
  // return {};

  TCPReceiverMessage message; // Create a TCPReceiverMessage to send
  if ( reassembler_.output_.has_error() ) {
    message.RST = true;
  }

  if ( reassembler_.SYN ) {
    uint64_t ackno = reassembler_.next_pushed_index();
    message.ackno = Wrap32::wrap( ackno, zero_point_ );
  }

  message.window_size = min( reassembler_.available_capacity(), (uint64_t)UINT16_MAX );

  return message;
}
