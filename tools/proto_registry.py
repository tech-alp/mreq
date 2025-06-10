#!/usr/bin/env python3
"""
proto_registry.py
- gen/ klasöründeki .pb.h dosyalarından topic_traits.hpp ve topic_registry.hpp üretir
"""
import os

GEN_DIR = "gen"
TRAITS_OUT = "src/mreq/topic_traits.hpp"
REGISTRY_OUT = "src/mreq/topic_registry.hpp"

# .pb.h dosyalarını tara
pb_headers = [f for f in os.listdir(GEN_DIR) if f.endswith(".pb.h")]

# Burada örnek olarak sadece dosya isimlerini yazıyoruz
with open(TRAITS_OUT, "w") as traits, open(REGISTRY_OUT, "w") as registry:
    traits.write("// Otomatik üretilen topic_traits.hpp\n")
    registry.write("// Otomatik üretilen topic_registry.hpp\n")
    for fname in pb_headers:
        msg_type = fname.replace(".pb.h", "")
        traits.write(f"// Trait: {msg_type}\n")
        registry.write(f"// Registry: {msg_type}\n")
    # TODO: Gerçek trait ve registry C++ kodu üretimi
