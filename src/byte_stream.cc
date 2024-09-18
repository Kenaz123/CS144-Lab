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
  size_t len = capacity_ - buffer_.size();
  size_t push_len = min(len, data.size());
  for (size_t i = 0; i < push_len; i++) {
    buffer_.push_back(data[i]);
  }
  write_cnt_ += push_len;
  (void)data;
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
  return capacity_ - buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return write_cnt_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return input_closed_ && buffer_.empty();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return read_cnt_;
}

string_view Reader::peek() const
{
  // Your code here.
  // string str = "";
  // str += buffer_[0];
  // return str;
  // 确保 buffer 不为空
    if (buffer_.empty()) {
        return ""; // 或者返回一个空的 string_view，取决于您的需求
    }
    
    // 返回指向缓冲区的第一个字节的视图
    return string_view(&buffer_.front(), 1); // 假设 buffer_ 是 std::vector或std::deque
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  if (len > bytes_buffered()) {
    set_error();
    return;
  }
  for (size_t i = 0; i < len; i++) {
    buffer_.pop_front();
  }
  read_cnt_ += len;
  (void)len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer_.size();
}
