#!/bin/sh
USER="$1"

if [ ! -d "debian" -a ${USER}x = "admin"x ]; then
	PKGNAME=${PWD##*/}
	VERSION="0.0.1"
	DEBNAME="${PKGNAME}_${VERSION}"
	echo "run dh_make for admin to generate debian files"
	# generate debian dir and default files
	dh_make -s --createorig -p "$DEBNAME"

	# create script from examples
	mv debian/postinst.ex debian/postinst
	mv debian/postrm.ex debian/postrm
	mv debian/preinst.ex debian/preinst
	mv debian/prerm.ex debian/prerm

	# create .gitignore for debian
	echo "!.gitignore" > debian/.gitignore
	echo "$PKGNAME/" > debian/.gitignore
	echo "$PKGNAME".debhelper.log >> debian/.gitignore
	echo "$PKGNAME".substvars >> debian/.gitignore
	echo "source/" >> debian/.gitignore

	# override the dh auto configure command
	echo "override_dh_auto_configure:" >> debian/rules
	echo "	./configure --enable-all" >> debian/rules

	# remove some files which we don't used
	rm -f ../"$DEBNAME".orig.tar.xz
	rm -f debian/README.Debian
	rm -f debian/README.source
	rm -f debian/docs
        rm -f debian/info
	rm -f debian/*.ex
	rm -f debian/*.EX
fi

if [ ! -d "debian" ]; then
	echo "can't found the debian directory, did you delete it?"
	exit
fi

dpkg-buildpackage -b
