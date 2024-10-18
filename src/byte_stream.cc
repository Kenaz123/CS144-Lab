#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), buffer_() {}

bool Writer::is_closed() const
{
  // Your code here.
  return Writer::input_closed_;
}

void Writer::push( string data )
{
  // Your code here.
  if ( available_capacity() == 0 ) {
    return;
  }
  size_t push_len = min( available_capacity(), data.size() );
  buffer_.append( data.begin(), data.begin() + push_len );
  used_ += push_len;
  write_cnt_ += push_len;
  return;
}

void Writer::close()
{
  // Your code here.
  Writer::input_closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - used_;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return write_cnt_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return input_closed_ && ( used_ == 0 );
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return read_cnt_;
}

string_view Reader::peek() const
{
  // Your code here.
  return static_cast<string_view>( this->buffer_ );
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  size_t pop_len = min( len, used_ );
  used_ -= pop_len;
  buffer_.erase( buffer_.begin(), buffer_.begin() + pop_len );
  read_cnt_ += pop_len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return used_;
}
