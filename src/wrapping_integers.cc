#include "wrapping_integers.hh"
#include <algorithm>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return Wrap32 { static_cast<uint32_t>( n ) + zero_point.raw_value_ };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  auto seqno_offset = raw_value_ - zero_point.raw_value_;
  auto bit1 = ( checkpoint + ( 1 << 31 ) ) & 0xFFFFFFFF00000000;
  auto bit2 = ( checkpoint - ( 1 << 31 ) ) & 0xFFFFFFFF00000000;
  auto res1 = seqno_offset | bit1;
  auto res2 = seqno_offset | bit2;
  if ( max( res1, checkpoint ) - min( res1, checkpoint ) <= max( res2, checkpoint ) - min( res2, checkpoint ) ) {
    return res1;
  }
  return res2;
}
