#!/bin/bash

for src in *.cc
do
	name=${src%.cc}
	make ${name}.so
	cp ${name}.so ../bin/modules/${name}
done
