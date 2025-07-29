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
    
    header_path = os.path.join(output_dir, 'topic_registry_autogen.hpp')
    impl_path = os.path.join(output_dir, 'topic_registry_autogen.cpp')

    topics = []
    include_files = set()

    for proto_file in proto_files:
        with open(proto_file, 'r') as pf:
            content = pf.read()
            message_match = re.search(r'message\s+(\w+)', content)
            if message_match:
                message_type = message_match.group(1)
                topic_names = extract_topic_names(content, message_type)
                
                proto_name = Path(proto_file).stem
                include_files.add(f'#include "{proto_name}.pb.h"')

                for topic_name in topic_names:
                    topics.append({
                        'type': message_type,
                        'name': topic_name,
                        'id': f'MREQ_ID({topic_name})'
                    })

    with open(header_path, 'w') as f:
        f.write("""#pragma once

namespace mreq::autogen {
void register_topics();
} // namespace mreq::autogen
""")

    with open(impl_path, 'w') as f:
        f.write('#include "topic_registry_autogen.hpp"\n')
        f.write('#include "mreq/mreq.hpp"\n')
        for include in sorted(list(include_files)):
            f.write(f'{include}\n')
        
        f.write("\nnamespace mreq::autogen {\n\n")
        f.write("void register_topics() {\n")
        for topic in topics:
            f.write(f'    mreq::register_topic<{topic["type"]}>({topic["id"]});\n')
        f.write("}\n\n")
        f.write("} // namespace mreq::autogen\n")

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