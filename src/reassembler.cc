#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // debug( "unimplemented insert({}, {}, {}) called", first_index, data, is_last_substring );

  // If the stream is closed, do nothing
  if ( data.empty() && is_last_substring ) {
    output_writer().close();
    return;
  }

  if ( is_last_substring ) {
    last_index_ = first_index + data.size() - 1;
  }

  // At least one byte of data is available to insert
  // next_pushed_index()返回滑动窗口的起始位置
  if ( first_index < next_pushed_index() + available_capacity()
       && first_index + data.length() > next_pushed_index() ) {
    // Calculate the inserted key
    uint64_t insert_key = ( first_index >= next_pushed_index() ) ? first_index : next_pushed_index();
    // Insert only the bytes within the available capacity
    pending_substrings_.insert( make_pair(
      insert_key,
      data.substr( insert_key - first_index,
                   min( data.length(), ( next_pushed_index() + available_capacity() ) - insert_key ) ) ) );
    // 怎么计算插入的字节数

    // Merge overlapping/adjacent substrings in the Reassembler's internal storage
    merge();

    // If next bytes are available, push these to the ByteStream
    // 如果下一个字节可用，则将其推送到 ByteStream
    if ( pending_substrings_.begin()->first == next_pushed_index() ) {
      output_writer().push( pending_substrings_.begin()->second );
      // Check if we have pushed the last byte of the stream
      if ( pending_substrings_.begin()->first + pending_substrings_.begin()->second.length() - 1 == last_index_ ) {
        output_writer().close();
      }
      pending_substrings_.erase( pending_substrings_.begin() );
    }
  }

  // debug( "discarding substring at index {} with size {}", first_index, data.size() );
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  // debug( "unimplemented count_bytes_pending() called" );

  uint64_t count = 0;
  for ( auto it = pending_substrings_.begin(); it != pending_substrings_.end(); ++it ) {
    count += it->second.size();
  }
  return count;
}

void Reassembler::merge()
{
  if ( pending_substrings_.size() <= 1 ) {
    return;
  }

  auto it = pending_substrings_.begin();
  auto next_it = std::next( it );
  while ( next_it != pending_substrings_.end() ) {
    if ( it->first + it->second.size() >= next_it->first ) {
      // Overlapping or adjacent substrings, merge them
      if ( it->first + it->second.size() >= next_it->first + next_it->second.size() ) {
        next_it = pending_substrings_.erase( next_it );
        // The first substring fully contains the second one
        continue;
      }
      auto overlap_pos = it->first + it->second.size() - next_it->first;
      it->second += next_it->second.substr( overlap_pos );
      next_it = pending_substrings_.erase( next_it );
    } else {
      // No overlap, move to the next pair
      it = next_it;
      ++next_it;
    }
  }
}
