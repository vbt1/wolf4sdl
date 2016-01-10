f:\d-tools\daemon -unmount 0
buildcd -msl.map -isl.img sl.cti -l
stripiso s 2048 sl.img sl.iso
f:\d-tools\daemon -mount 0,sl.iso


