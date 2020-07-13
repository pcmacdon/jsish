#!/bin/bash
# Test all .jsi scripts in system.
for i in `find . -name "*.jsi"`; do
    if echo $i | fgrep -q /html/; then
        #echo $i
        continue
    fi
    if echo $i | fgrep -q /warn; then
        #echo $i
        continue
    fi
    if echo $i | fgrep -q /tests/; then
        #echo $i
        continue
    fi
    jsish --I noEval=true $i
done
