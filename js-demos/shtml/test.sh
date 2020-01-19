#!/bin/sh

JSW="jsish -W -timeout 1000"

case $1 in
     "1") jsish -W -timeout 1000 -query 'NOPLUGIN=true&ALTPLUGIN=true&OPT1=true' index.shtml ;;
     "2") jsish -W -timeout 1000 -udata '{OPT1:true}' -query 'NOPLUGIN=true&ALTPLUGIN=true' index.shtml ;;
     "3") jsish -W -timeout 1000 -query 'NOPLUGIN=false' index2.shtml ;;
     "4") jsish -W -timeout 1000 -query 'NOPLUGIN=true&ALTPLUGIN=false' index2.shtml ;;
     "5") jsish -W -timeout 1000 -query 'NOPLUGIN=true&ALTPLUGIN=false' index3.shtml ;;
     "6") jsish -W -timeout 1000 -query 'NOPLUGIN=true&ALTPLUGIN=true' index3.shtml ;;
     "7") jsish -W -timeout 1000 -query 'NOPLUGIN=false&ALTPLUGIN=true' index3.shtml ;;
     *) echo "Run with arg 1-n"; exit 1;;
esac

