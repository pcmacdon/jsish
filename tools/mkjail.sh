#!/bin/sh
# Create and mount jail for jsish, or unmount if exists
if [ `whoami` != "root" ]; then
  echo "must run as root or use sudo"
  exit 1
fi

DST=home001
if [ "$1" != "" ]; then
  DST=$1
fi
FMNT=`findmnt $DST`
if [ "$FMNT" != "" ]; then
    echo "Unmounting $DST"
    umount $DST/bin
    umount $DST/bin
    umount $DST
    exit 0;
fi
echo Mounting $DST
if [ ! -d bin ]; then
    mkdir bin 
    mknod -m 0666 bin/null c 1 3 
    mknod -m 0666 bin/random c 1 8
    mknod -m 0666 bin/urandom c 1 9 
    cat <<EOF > bin/hosts
127.0.0.1       localhost
EOF
    cat <<EOF > bin/resolv.conf 
nameserver 127.0.1.1
search local
EOF
fi

mkdir -p $DST
(cd $DST
  mkdir -p bin
  ln -sf bin etc
  ln -sf bin dev
)
mount --bind $DST $DST
mount --bind bin $DST/bin
mount -o remount,nodev,noexec,nosuid $DST $DST
mount -o bind,ro bin $DST/bin

findmnt | fgrep [
exit 0
#
