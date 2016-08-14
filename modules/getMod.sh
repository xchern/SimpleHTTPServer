#!/bin/bash

[[ -d ../bin/ ]] || mkdir ../bin
[[ -d ../bin/modules/ ]] || mkdir ../bin/modules

for src in *.cc
do
	name=${src%.cc}
	make ${name}.so
	cp ${name}.so ../bin/modules/${name}
done
