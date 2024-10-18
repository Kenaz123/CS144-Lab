#include "reassembler.hh"
#include <stdexcept>

using namespace std;

// Insert a new substring to be reassembled into a ByteStream.
void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // Your code here.
  if ( first_index >= cur_index_ + output_.writer().available_capacity() ) {
    return; // 超出可用容量，丢弃数据
  }
  uint64_t insert_end = first_index + data.size();
  if ( is_last_substring ) {
    eof_index_ = insert_end;
  }
  // 截取超出部分
  insert_end = min( insert_end, cur_index_ + output_.writer().available_capacity() );
  // 插入或合并区间
  // 若insert_end < cur_index，说明数据已经被丢弃，不需要插入
  if ( insert_end > cur_index_ ) {
    auto it = pending_data_.lower_bound( first_index );
    if ( it != pending_data_.begin() && prev( it )->first + prev( it )->second.size() >= first_index ) {
      --it;
    }
    // string new_data = data.substr(0, insert_end - first_index);
    uint64_t insert_start = max( cur_index_, first_index );
    string new_data = data.substr( insert_start - first_index, insert_end - insert_start );
    while ( it != pending_data_.end() && it->first <= insert_end ) {
      if ( it->first < first_index ) {
        insert_start = it->first;
        new_data = it->second.substr( 0, first_index - it->first ) + new_data;
      }
      if ( it->first + it->second.size() > insert_end ) {
        new_data += it->second.substr( insert_end - it->first, it->second.size() - ( insert_end - it->first ) );
      }
      auto tmp = it;
      ++it;
      pending_data_.erase( tmp );
    }
    pending_data_[insert_start] = new_data;
  }
  // 输出可以写入的部分
  auto writable_it = pending_data_.find( cur_index_ );
  while ( writable_it != pending_data_.end() && writable_it->first == cur_index_ ) {
    output_.writer().push( writable_it->second );
    cur_index_ += writable_it->second.size();
    writable_it = pending_data_.erase( writable_it );
  }

  // 判断是否应关闭输出流
  if ( cur_index_ == eof_index_ ) {
    output_.writer().close();
  }
}

// How many bytes are stored in the Reassembler itself?
uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  // 计算出pending_data_中所有字符串的长度之和
  uint64_t res = 0;
  for ( auto& p : pending_data_ ) {
    res += p.second.size();
  }
  return res;
}
// uint64_t st = max( first_index, cur_index_ );
// uint64_t ed = min( first_index + data.size(),min( eof_index_, cur_index_ + output_.writer().available_capacity()
// ) ); if(is_last_substring){
//   eof_index_ = min(eof_index_, first_index + data.size());
// }
// for(uint64_t i = st; i < ed; i++){
//   uint64_t j = i - first_index;
//   auto &p = pending_data_[i];
//   if(p.second){
//     continue;
//   } else {
//     p.first = data[j];
//     p.second = true;
//     pending_bytes_++;
//   }
// }
// string str;
// uint64_t prev = cur_index_;
// pair<char,bool> t;
// while(cur_index_ < eof_index_ && (t = pending_data_[cur_index_]).second){
//   str.push_back(t.first);
//   pending_bytes_--;
//   cur_index_++;
// }
// output_.writer().push(str);
// pending_data_.erase(pending_data_.find(prev), pending_data_.find(cur_index_));
// if(cur_index_ == eof_index_){
//   output_.writer().close();
// }