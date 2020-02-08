#!/bin/sh

#build yariv first and put yariv_pack binary to this folder

python yariv_to_hex.py src/main.vert
python yariv_to_hex.py src/main.frag

python yariv_to_hex.py src/buf.vert
python yariv_to_hex.py src/buf.frag
