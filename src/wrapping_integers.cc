#include "wrapping_integers.hh"
#include "debug.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  // debug( "unimplemented wrap( {}, {} ) called", n, zero_point.raw_value_ );
  // return Wrap32 { 0 };

  return zero_point + n;
  // n implicitly convert to uint32_t in order to match the signature of operator+
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // debug( "unimplemented unwrap( {}, {} ) called", zero_point.raw_value_, checkpoint );
  // return {};

  static constexpr uint64_t int31 = 1UL << 31;
  static constexpr uint64_t int32 = 1uL << 32;

  auto const ckp32 = wrap( checkpoint, zero_point );
  auto const diff = raw_value_ - ckp32.raw_value_;

  if ( diff <= int31 || checkpoint + diff < int32 )
    return checkpoint + diff;
  return checkpoint + diff - int32;
  // This unwrap method returns the absolute sequence number that wraps to this Wrap32,
}
