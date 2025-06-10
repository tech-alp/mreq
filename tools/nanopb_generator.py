#!/usr/bin/env python3
"""
nanopb_generator.py
- proto/ klasöründeki tüm .proto dosyalarını gen/ klasörüne nanopb ile derler
"""
import os
import subprocess

PROTO_DIR = "proto"
GEN_DIR = "../gen"  # Çalışma dizini proto/ olacağı için bir üstteki gen/

# nanopb'nin protoc plugin yolunu buraya ekleyin
def get_nanopb_plugin():
    return "/opt/homebrew/bin/protoc-gen-nanopb"

os.makedirs("gen", exist_ok=True)

for fname in os.listdir(PROTO_DIR):
    if fname.endswith(".proto"):
        proto_path = os.path.join(PROTO_DIR, fname)
        cmd = [
            "protoc",
            f"--plugin=protoc-gen-nanopb={get_nanopb_plugin()}",
            f"--nanopb_out={GEN_DIR}",
            f"--proto_path=.",
            fname
        ]
        print("Çalıştırılıyor:", " ".join(cmd))
        subprocess.run(cmd, check=True, cwd=PROTO_DIR)
