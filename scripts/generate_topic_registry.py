#!/usr/bin/env python3
import os
import re
import sys
from pathlib import Path

def extract_topic_names(proto_content, message_name):
    """Extract topic names from proto file comments or use message name."""
    topic_comment = re.search(r'//\s*@topic\s*:\s*([^\n]+)', proto_content)
    if topic_comment:
        return topic_comment.group(1).strip().split()
    return [message_name]

def generate_registry_code(proto_files, output_dir):
    """Generate topic registry code from proto files."""
    os.makedirs(output_dir, exist_ok=True)

    hpp_path = os.path.join(output_dir, 'topic_registry_autogen.hpp')
    cpp_path = os.path.join(output_dir, 'topic_registry_autogen.cpp')

    proto_info_list = []
    for proto_file in proto_files:
        with open(proto_file, 'r') as pf:
            content = pf.read()
            message_match = re.search(r'message\s+(\w+)', content)
            if message_match:
                message_type = message_match.group(1)
                topic_names = extract_topic_names(content, message_type)
                proto_info_list.append({
                    "file_path": proto_file,
                    "message_type": message_type,
                    "topic_names": topic_names
                })

    # Generate header file
    with open(hpp_path, 'w') as f:
        f.write("""#pragma once

#include "mreq/metadata.hpp"
""")
        for proto_info in proto_info_list:
            proto_name = Path(proto_info["file_path"]).stem
            f.write(f'#include <{proto_name}.pb.h>\n')
        f.write("""
namespace mreq {
namespace autogen {

""")
        for proto_info in proto_info_list:
            f.write(f'MREQ_METADATA_DECLARE({proto_info["message_type"]}, "{proto_info["message_type"]}")\n')
        f.write("""
void register_topics();

} // namespace autogen
} // namespace mreq
""")

    # Generate implementation file
    with open(cpp_path, 'w') as f:
        f.write("""#include "topic_registry_autogen.hpp"
#include "mreq/topic_registry.hpp"

namespace mreq {
namespace autogen {

""")
        for proto_info in proto_info_list:
            # Her mesaj tipi için sadece bir metadata instance, mesaj ismiyle.
            f.write(f'MREQ_MESSAGE_TYPE({proto_info["message_type"]}, "{proto_info["message_type"]}")\n')
        f.write("""
void register_topics() {
""")
        for proto_info in proto_info_list:
            for topic_name in proto_info["topic_names"]:
                f.write(f'    REGISTER_TOPIC({proto_info["message_type"]}, "{topic_name}");\n')
        f.write("""}

} // namespace autogen
} // namespace mreq
""")

def main():
    if len(sys.argv) < 3:
        print("Usage: generate_topic_registry.py <proto_file1> [proto_file2 ...] <output_dir>")
        sys.exit(1)

    # Get proto files from command line arguments (all except last)
    proto_files = sys.argv[1:-1]
    # Get output directory (last argument)
    output_dir = sys.argv[-1]

    for proto_file in proto_files:
        if not os.path.exists(proto_file):
            print(f"Error: Proto file not found: {proto_file}")
            sys.exit(1)
    
    generate_registry_code(proto_files, output_dir)

if __name__ == '__main__':
    main()