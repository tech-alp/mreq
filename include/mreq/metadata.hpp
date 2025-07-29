#pragma once

#include <cstddef> // For size_t

namespace mreq {

// Placeholder function pointer types for serialization/deserialization
using serialize_fn_t = bool (*)(const void* msg_in, size_t size, void* buffer_out);
using deserialize_fn_t = bool (*)(const void* buffer_in, size_t size, void* msg_out);

struct mreq_metadata {
    const char* const topic_name;
    const size_t payload_size;
    const serialize_fn_t serialize;
    const deserialize_fn_t deserialize;
};

// The MREQ_ID macro resolves to the address of the static metadata object
#define MREQ_ID(name) (&::mreq::autogen::__mreq_##name)

} // namespace mreq

// Forward declare the namespace for generated metadata
namespace mreq::autogen {}