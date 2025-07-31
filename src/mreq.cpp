#include "mreq/mreq.hpp"
#include "mreq/topic_registry.hpp"
#include <optional>
#include "mreq/mreq.hpp"
#include "mreq/topic.hpp"
#include <optional>

namespace mreq {

void init() {

}

} // namespace mreq

#include "mreq/metadata.hpp"
#include "pb_encode.h"
#include "pb_decode.h"

namespace mreq {

bool nanopb_encode_wrapper(const mreq_metadata& metadata, const void* data, void* buffer, size_t buffer_size, size_t* message_length) {
    if (!metadata.fields) return false;

    pb_ostream_t stream = pb_ostream_from_buffer(static_cast<pb_byte_t*>(buffer), buffer_size);
    
    if (!pb_encode(&stream, metadata.fields, data)) {
        return false;
    }
    
    *message_length = stream.bytes_written;
    return true;
}

bool nanopb_decode_wrapper(const mreq_metadata& metadata, const void* buffer, size_t buffer_size, void* data) {
    if (!metadata.fields) return false;

    pb_istream_t stream = pb_istream_from_buffer(static_cast<const pb_byte_t*>(buffer), buffer_size);
    
    return pb_decode(&stream, metadata.fields, data);
}

} // namespace mreq
