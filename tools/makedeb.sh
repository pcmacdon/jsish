#!/bin/bash
# A quick hack to generate a Debian package of Jsi. Taken from jsish, and
# from Martin Krafft's "The Debian System" book.

DEB_REV=${1-1} # .deb package build/revision number.
PACKAGE_DEBNAME=jsish
THISDIR=${PWD}

if uname -a | grep -i nexenta &>/dev/null; then
# Assume NexentaOS/GnuSolaris:
    DEB_ARCH_NAME=solaris-i386
    DEB_ARCH_PKGDEPENDS="sunwcsl" # for -lsocket
else
    DEB_ARCH_NAME=$(dpkg --print-architecture)
fi

SRCDIR=$(cd ..; pwd)
test -e ${SRCDIR}/jsish || {
    echo "This script must be run from a BUILT copy of the source tree."
    exit 1
}

DEBROOT=$PWD/deb.tmp
test -d ${DEBROOT} && rm -fr ${DEBROOT}

DEBLOCALPREFIX=${DEBROOT}/usr
BINDIR=${DEBLOCALPREFIX}/bin
mkdir -p ${BINDIR}
mkdir -p ${DEBLOCALPREFIX}/share/doc/${PACKAGE_DEBNAME}
cp ../jsish ${BINDIR}
strip ${BINDIR}/jsish

cd $DEBROOT || {
    echo "Debian dest dir [$DEBROOT] not found. :("
    exit 2
}


rm -fr DEBIAN
mkdir DEBIAN

PACKAGE_VERSION=$(date +%Y.%m.%d)
PACKAGE_DEB_VERSION=${PACKAGE_VERSION}-${DEB_REV}
DEBFILE=${THISDIR}/${PACKAGE_DEBNAME}-${PACKAGE_DEB_VERSION}-dev-${DEB_ARCH_NAME}.deb
PACKAGE_TIME=$(/bin/date)

rm -f ${DEBFILE}
echo "Creating .deb package [${DEBFILE}]..."

echo "Generating md5 sums..."
find ${DEBLOCALPREFIX} -type f -exec md5sum {} \; > DEBIAN/md5sums

true && {
    echo "Generating Debian-specific files..."
    COPYRIGHT=${DEBLOCALPREFIX}/share/doc/${PACKAGE_DEBNAME}/copyright
    cat <<EOF > ${COPYRIGHT}
This package was created by jsish-scm <jsish@jsish.org>
on ${PACKAGE_TIME}.

The original sources for jsish can be downloaded for free from:

http://www.jsish-scm.org/

jsish is released under the terms of the GNU General Public License.

EOF
}

true && {
    CHANGELOG=${DEBLOCALPREFIX}/share/doc/${PACKAGE_DEBNAME}/changelog.gz
    cat <<EOF | gzip -c > ${CHANGELOG}
${PACKAGE_DEBNAME} ${PACKAGE_DEB_VERSION}; urgency=low

This release has no changes over the core source distribution. It has
simply been Debianized.

Packaged by jsish-dev <jsish@jsish.org> on
${PACKAGE_TIME}.

EOF

}


true && {
    CONTROL=DEBIAN/control
    echo "Generating ${CONTROL}..."
    cat <<EOF > ${CONTROL}
Package: ${PACKAGE_DEBNAME}
Section: vcs
Priority: optional
Maintainer: jsish-dev <jsish@jsish.org>
Architecture: ${DEB_ARCH_NAME}
Depends: libc6 ${DEB_ARCH_PKGDEPENDS+, }${DEB_ARCH_PKGDEPENDS}
Version: ${PACKAGE_DEB_VERSION}
Description: Jsish is a javascript shell for embedded development.
 This package contains the Jsish binary for *buntu/Debian systems.
 Jsish home page: http://jsish.org
 Fossil author: Peter MacDonald
 License: GNU GPLv2
EOF

}


true && {
#    GZ_CONTROL=control.tar.gz
#    GZ_DATA=data.tar.gz
#    echo "Generating ${GZ_CONTROL} and ${GZ_DATA}..."
#    rm -f ${GZ_CONTROL} ${GZ_DATA} ${DEBFILE} 2>/dev/null
#    tar cz -C DEBIAN -f ${GZ_CONTROL} .
#    tar czf ${GZ_DATA} --exclude='*/doxygen-*' usr
#    echo '2.0' > debian-binary
    #ar rcu ${DEBFILE} debian-binary ${GZ_CONTROL} ${GZ_DATA}
    dpkg-deb -b ${DEBROOT} ${DEBFILE}
    echo "Package file created:"
    ls -la ${DEBFILE}
    dpkg-deb --info ${DEBFILE}
}

cd - >/dev/null
true && {
    echo "Cleaning up..."
    rm -fr ${DEBROOT}
}

echo "Done :)"
