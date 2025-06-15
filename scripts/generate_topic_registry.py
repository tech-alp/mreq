#!/usr/bin/env python3
import os
import re
import sys
from pathlib import Path

def extract_topic_name(proto_content, proto_filename):
    """Extract topic name from proto file comments or use filename."""
    # Look for topic name in comments
    topic_comment = re.search(r'//\s*@topic\s*:\s*([^\n]+)', proto_content)
    if topic_comment:
        return topic_comment.group(1).strip()
    
    # If no comment found, use filename without extension
    return Path(proto_filename).stem

def generate_registry_code(proto_files, output_dir):
    """Generate topic registry code from proto files."""
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    # Generate header file
    header_path = os.path.join(output_dir, 'topic_registry_autogen.hpp')
    with open(header_path, 'w') as f:
        f.write("""#pragma once
#include <string>
#include "mreq/interface.hpp"

namespace mreq {
namespace autogen {

// Topic registrations
""")
        
        # Add topic registrations for each proto file
        for proto_file in proto_files:
            with open(proto_file, 'r') as pf:
                content = pf.read()
                message_match = re.search(r'message\s+(\w+)', content)
                if message_match:
                    message_type = message_match.group(1)
                    topic_name = extract_topic_name(content, proto_file)
                    f.write(f'REGISTER_TOPIC({message_type}, "{topic_name}")\n')
        
        f.write("""
} // namespace autogen
} // namespace mreq
""")

    # Generate implementation file
    impl_path = os.path.join(output_dir, 'topic_registry_autogen.cpp')
    with open(impl_path, 'w') as f:
        f.write("""#include "topic_registry_autogen.hpp"
""")
        
        # Add includes for all proto files
        for proto_file in proto_files:
            proto_name = Path(proto_file).stem
            f.write(f'#include <{proto_name}.pb.h>\n')
        
        f.write("""
namespace mreq {
namespace autogen {

// Topic registrations
""")
        
        # Add topic registrations for each proto file
        for proto_file in proto_files:
            with open(proto_file, 'r') as pf:
                content = pf.read()
                message_match = re.search(r'message\s+(\w+)', content)
                if message_match:
                    message_type = message_match.group(1)
                    topic_name = extract_topic_name(content, proto_file)
                    f.write(f'REGISTER_TOPIC({message_type}, "{topic_name}")\n')
        
        f.write("""
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