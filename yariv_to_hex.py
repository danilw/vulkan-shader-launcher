import struct
import os
import sys
import subprocess


if len(sys.argv) != 2:
  print('Usage: python %s filename' % sys.argv[0])
  quit()

inputfilepath = sys.argv[1]
outputname = os.path.basename(inputfilepath)
outdir = os.path.dirname(inputfilepath)

spirvcompiler = 'glslangValidator'
if os.name == 'nt':
  spirvcompiler += ".exe"

yariv_pack = '../yariv_pack'
if os.name == 'nt':
  spirvcompiler += ".exe"

subprocess.call([spirvcompiler,'-V',inputfilepath,'-o',inputfilepath + '.spv'])

subprocess.call([yariv_pack,inputfilepath + '.spv'])

infile = open(inputfilepath + '.yariv', 'rb')
outfilepath = os.path.join(outdir,outputname + '.hex')
outfile = open(outfilepath, 'w')


lineno = 1
while 1 :
  b = infile.read(1)
  if len(b) == 0 :
    break
  d, = struct.unpack('B', b)
  outfile.write(hex(d) + ',')
  if lineno % 20 == 0:
    outfile.write('\n')
  lineno = lineno + 1
