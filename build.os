#!/bin/bash

set -e
set -o pipefail

MINPARAMS=1
if [ $# -lt "$MINPARAMS" ]
then
    echo usage:
    echo   build.os  "wr | nos | linux | lxde" "target"
    echo
    echo i.e.:
    echo   build.os nos x86_64-unknown-linux-gnu
    echo
    exit 1
fi

BUILDTYPE=$1
WR=0
NOS=0
LINUX=0
LXDE=0
OS=""
TARGET=$2
LIBC=""
USENEWTOOL=1

case "$BUILDTYPE" in
    wr)
        echo "WR Build"
        WR=1
        OS="wr"
        LIBC="uclibc"
        ;;
    nos)
        echo "NOS Build"
        NOS=1
        OS="nos"
        LIBC="eglibc"
        ;;
    linux)
        echo "Linux Build"
        LINUX=1
        OS="linux"
        LIBC="glibc"
        ;;
    lxde)
        echo "LXDE Build"
        LXDE=1
        OS="lxde"
        LIBC="eglibc"
        ;;
    *)
        echo "Choices are: wr, linux, nos, lxde"
        exit 1
        ;;
esac

TOP=`pwd`
echo Source Base=$TOP

ulimit -c unlimited

source $TOP/packages_definitions

echo "----------Testing Build Tool Versions----------"

$TOP/Config/buildpackageversion

echo "----------Printing Environment Variables-------"

$TOP/Config/envtest

echo "Clean directories"

rm -rf $TOP/buildoutput/$OS
mkdir $TOP/buildoutput/$OS
rm -rf $TOP/images/$OS
mkdir $TOP/images/$OS
mkdir $TOP/images/$OS/etc
cp $TOP/boot/etc/ld.so.conf $TOP/images/$OS/etc/ld.so.conf
mkdir $TOP/images/$OS/etc/ld.so.conf.d/

for pack in $nos_packages; do
    cd $TOP/packages/$pack
    case "$pack" in
    iana-etc)
        if [ -f Makefile ]
        then
            make clean
        fi
        ;;
        
    openssl)
        if [ -f Makefile ]
        then
            make clean
        fi
        ;;
        
    procps)
        if [ -f Makefile ]
        then
            make clean
        fi
        ;;
        
    mpc)
        if [ -f Makefile ]
        then
            make clean
        fi
        ;;
        
    mpfr)
        if [ -f Makefile ]
        then
            make clean
        fi
        ;;
        
    acl)
        if [ -f Makefile ]
        then
            make clean
        fi
        ;;
        
    debconf)
        if [ -f Makefile ]
        then
            make clean
        fi
        ;;
        
    attr)
        if [ -f Makefile ]
        then
            make clean
        fi
        ;;
        
    *)
        if [ -f Makefile ]
        then
            make distclean
        fi
        ;;
    esac
    cd $TOP
done

for pack in $lxde_packages; do
    cd $TOP/lxde/$pack
    case "$pack" in
    *)
        if [ -f Makefile ]
        then
            make distclean
        fi
        ;;
    esac
    cd $TOP
done

for pack in $linux_packages; do
    cd $TOP/packages/$pack
    case "$pack" in
    ppp)
        if [ -f config.h ]
        then
            make clean
        fi
        ;;
    ppp)
        if [ -f Makefile ]
        then
            make clean
        fi
        ;;
    *)
        if [ -f Makefile ]
        then
            make distclean
        fi
        ;;
    esac
    cd $TOP
done

for pack in $tpd_gnome_packages; do
    cd $TOP/tpd/display/$pack
    case "$pack" in
    libical)
        if [ -f Makefile ]
        then
            make clean
        fi
        ;;
    *)
        if [ -f Makefile ]
        then
            make distclean
        fi
        ;;
    esac
    cd $TOP
done

for pack in $tpd_X_packages; do
    cd $TOP/tpd/display/$pack
    case "$pack" in
    *)
        if [ -f Makefile ]
        then
            make distclean
        fi
        ;;
    esac
    cd $TOP
done

for pack in $wr_os_linux_packages; do
    cd $TOP/packages/$pack
    case "$pack" in
    iana-etc)
        if [ -f Makefile ]
        then
            make clean
        fi
        ;;
        
    openssl)
        if [ -f Makefile ]
        then
            make clean
        fi
        ;;
        
    *)
        if [ -f Makefile ]
        then
            make distclean
        fi
        ;;
    esac
    cd $TOP
done

echo "----------Starting CbirLinux Build-------------"

export PREFIX=$TOP/images/$OS
export LINUX_INSTALL_DIR=$TOP/images/$OS

function install_boot_scripts {
cd $TOP/boot/bootscripts
make DESTDIR=$LINUX_INSTALL_DIR install
ln -s $LINUX_INSTALL_DIR/bin/bash $LINUX_INSTALL_DIR/bin/sh
ln -s $LINUX_INSTALL_DIR/sbin/killall5 $LINUX_INSTALL_DIR/sbin/killall
cp -r $TOP/boot/bootscripts/systemd $LINUX_INSTALL_DIR/etc/systemd
}

function install_locale {

echo "Installing Locale"
LOCALEDEF=$PREFIX/usr/bin/localedef
LOCALES=$TOP/packages/$LIBC/localedata/locales
CHARMAPS=$TOP/packages/$LIBC/localedata/charmaps
$LOCALEDEF -i $LOCALES/cs_CZ -f $CHARMAPS/UTF-8 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/de_DE -f $CHARMAPS/ISO-8859-1 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/de_DE@euro -f $CHARMAPS/ISO-8859-15 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/de_DE -f $CHARMAPS/UTF-8 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/en_GB -f $CHARMAPS/UTF-8 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/en_HK -f $CHARMAPS/ISO-8859-1 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/en_PH -f $CHARMAPS/ISO-8859-1 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/en_US -f $CHARMAPS/ISO-8859-1 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/en_US -f $CHARMAPS/UTF-8 $PREFIX/usr/lib/locale/en_US.UTF-8 --prefix=$PREFIX --add-to-archive
$LOCALEDEF -i $LOCALES/es_MX -f $CHARMAPS/ISO-8859-1 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/fa_IR -f $CHARMAPS/UTF-8 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/fr_FR -f $CHARMAPS/ISO-8859-1 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/fr_FR@euro -f $CHARMAPS/ISO-8859-15 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/fr_FR -f $CHARMAPS/UTF-8 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/it_IT -f $CHARMAPS/ISO-8859-1 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/it_IT -f $CHARMAPS/UTF-8 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/ja_JP -f $CHARMAPS/EUC-JP --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/ru_RU -f $CHARMAPS/KOI8-R --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/ru_RU -f $CHARMAPS/UTF-8 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/tr_TR -f $CHARMAPS/UTF-8 --prefix=$PREFIX --add-to-archive 
$LOCALEDEF -i $LOCALES/zh_CN -f $CHARMAPS/GB18030 --prefix=$PREFIX --add-to-archive 
}

function install_timezone {
cd $1
tar -xvf $TOP/boot/tzdata2013c.tar.gz
mkdir -pv $1/{posix,right}
for tz in etcetera southamerica northamerica europe africa antarctica \
    asia australasia backward pacificnew solar87 solar88 solar89 \
    systemv; do
    zic -L /dev/null -d $1 -y "sh yearistype.sh" ${tz}
    zic -L /dev/null -d $1/posix -y "sh yearistype.sh" ${tz}
    zic -L leapseconds -d $1/right -y "sh yearistype.sh" ${tz}
done
zic -d $1 -p Asia/Kolkata
cd $TOP/buildoutput/$OS/$LIBC
}

function set_build_env {
export PATH=$PREFIX/bin:$PREFIX/usr/bin:$PREFIX/sbin:$PREFIX/usr/sbin:$TOP/$TARGET/usr/sbin:$TOP/$TARGET/usr/bin:$TOP/$TARGET/sbin:$TOP/$TARGET/bin:$TOP/x86_64-unknown-linux-gnu/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games
export PKG_CONFIG_PATH=$TOP/$TARGET/lib/pkgconfig:$TOP/$TARGET/lib64/pkgconfig:$PREFIX/lib/pkgconfig:$PREFIX/lib64/pkgconfig:$PREFIX/usr/lib/pkgconfig:$PREFIX/usr/lib64/pkgconfig
export LD_LIBRARY_PATH=$TOP/$TARGET/lib:$TOP/$TARGET/lib64:$TOP/$TARGET/usr/lib:$TOP/$TARGET/usr/lib64:$TOP/$TARGET/libexec:$TOP/x86_64-unknown-linux-gnu/lib:$TOP/$TARGET/x86_64-unknown-linux-gnu/lib64:$PREFIX/lib:$PREFIX/lib64:$PREFIX/usr/lib:$PREFIX/libexec:$PREFIX/usr/lib64
export C_INCLUDE_PATH=$TOP/$TARGET/include:$TOP/$TARGET/usr/include:$PREFIX/include:$PREFIX/usr/include:$TOP/$TARGET/x86_64-unknown-linux-gnu/include
export LIBRARY_PATH=$TOP/$TARGET/lib:$TOP/$TARGET/lib64:$TOP/$TARGET/usr/lib:$TOP/$TARGET/usr/lib64:$TOP/$TARGET/libexec:$TOP/x86_64-unknown-linux-gnu/lib:$TOP/$TARGET/x86_64-unknown-linux-gnu/lib64:$PREFIX/lib:$PREFIX/lib64:$PREFIX/usr/lib:$PREFIX/libexec:$PREFIX/usr/lib64
export CXX_INCLUDE_PATH=$PREFIX/include:$PREFIX/usr/include:$TOP/$TARGET/include:$TOP/$TARGET/usr/include:$TOP/$TARGET/x86_64-unknown-linux-gnu/include
export CFLAGS="-fPIC -I$PREFIX/include -I$PREFIX/usr/include -I$TOP/include -I$TOP/usr/include -I$TOP/$TARGET/x86_64-unknown-linux-gnu/include"
export CXXFLAGS="-fPIC -I$PREFIX/include -I$PREFIX/usr/include -I$TOP/include -I$TOP/usr/include -I$TOP/$TARGET/x86_64-unknown-linux-gnu/include"
export LDFLAGS="-L$PREFIX/lib -L$PREFIX/lib64 -L$PREFIX/usr/lib -L$PREFIX/libexec -L$PREFIX/usr/lib64 -L$TOP/$TARGET/lib -L$TOP/$TARGET/lib64 -L$TOP/$TARGET/usr/lib -L$TOP/$TARGET/usr/lib64 -L$TOP/$TARGET/x86_64-unknown-linux-gnu/lib" 
}

function libtool_fixup {
    sed -i -e "s_ /lib_ $PREFIX/lib_g" -e "s_'/lib_'$PREFIX/lib_g" $1
}

function build_wr_os_linux_component {

    echo "----------------------------------------------------------"
    echo "------------------- building $1 --------------------------"
    echo "----------------------------------------------------------"

    DIR=$2
    case "$1" in
    uclibc)
    cd $TOP/packages/uclibc
    if [ ARCH = "mips" ]
        cp .config_mips .config
    else if [ ARCH = "arm" ]
        cp .config_arm .config
    fi
    make TARGET_ARCH=$ARCH oldconfig
    make TARGET_ARCH=$ARCH all
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    busybox)
    cd $TOP/packages/busybox
    if [ ARCH = "mips" ]
        cp .config_mips .config
    else if [ ARCH = "arm" ]
        cp .config_arm .config
    fi
    make TARGET_ARCH=$ARCH oldconfig
    make TARGET_ARCH=$ARCH all
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    openssl)
    rm -rf $TOP/buildoutput/$DIR/openssl
    mkdir $TOP/buildoutput/$DIR/openssl
    cd $TOP/buildoutput/$DIR/openssl
    export OPENSSL_SOURCE=$TOP/packages/openssl
    $TOP/Config/openssl_tree
    ./Configure --prefix=/usr linux-$ARCH \
            -L$PREFIX/lib shared -I$PREFIX/include -fPIC
    make
    make INSTALL_PREFIX=$PREFIX install
    make clean
    ;;

    curl)
    rm -rf $TOP/buildoutput/$DIR/curl
    mkdir $TOP/buildoutput/$DIR/curl
    cd $TOP/buildoutput/$DIR/curl
    ../../../packages/curl/configure --prefix=/usr \
        --with-pic LIBS="-lssl -lcrypto" --disable-manual \
        --disable-debug
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/usr/lib/libcurl.la
    make distclean
    ;;

    iana-etc)
    cd $TOP/packages/iana-etc
    make DESTDIR=$PREFIX install
    make clean
    export DESTDIR=
    ;;

    sysvinit)
    cd $TOP/packages/sysvinit
    make -C src 
    make -C src ROOT=$PREFIX install
    mv $PREFIX/sbin/init $PREFIX/sbin/init.sysv
    make clean
    ;;

    iptables)
    rm -rf $TOP/buildoutput/$DIR/iptables
    mkdir $TOP/buildoutput/$DIR/iptables
    cd $TOP/buildoutput/$DIR/iptables
    ../../../packages/iptables/configure --prefix=/usr \
        --with-pic LIBS="-lssl -lcrypto" --disable-manual \
        --disable-debug
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    iproute2)
    cd $TOP/packages/iproute2
    make
    make BINDIR=$TOP/images/$DIR/bin \
         MANDIR=$TOP/images/$DIR/share/man/man5 \
         PPPDIR=$TOP/images/$DIR/etc/ppp/peers install
    make distclean
    ;;

    snortsam)
    cd $TOP/packages/snortsam
    ./makesnortsam.sh
    cp snortsam $PREFIX/bin
    ;;

    wpa_supplicant)
    cd $TOP/packages/wpa_supplicant/wpa_supplicant
    cp .config_full .config
    make
    make DESTDIR=$PREFIX install
    make clean
    export DESTDIR=
    ;;

    wvstreams)
    cd $TOP/cce/wvstreams
    ./configure --prefix=
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    wvdial)
    cd $TOP/cce/wvdial
    ./configure
    make
    make BINDIR=$TOP/images/$DIR/bin \
         MANDIR=$TOP/images/$DIR/share/man/man5 \
         PPPDIR=$TOP/images/$DIR/etc/ppp/peers install
    make distclean
    ;;

    *)
    echo "Wrong $DIR pack"
    exit 1
    ;;
    esac
}

function build_nos_linux_component {

    echo "----------------------------------------------------------"
    echo "------------------- building $1 --------------------------"
    echo "----------------------------------------------------------"

    DIR=$2
    case "$1" in
    acl)
    rm -rf $TOP/buildoutput/$DIR/acl
    mkdir $TOP/buildoutput/$DIR/acl
    cd $TOP/buildoutput/$DIR/acl
    export PKG_CFLAGS=-I$PREFIX/include
    export ACL_SOURCE=$TOP/packages/acl
    $TOP/Config/acl_tree
    ./configure --prefix=/usr --with-pic --enable-lib64=yes \
          --enable-static=no --enable-gettext=no
    make
    make DIST_ROOT=$PREFIX install install-dev install-lib
    libtool_fixup $PREFIX/usr/libexec64/libacl.la
    make clean
    export ACL_SOURCE=
    export PKG_CFLAGS=
    ;;

    attr)
    rm -rf $TOP/buildoutput/$DIR/attr
    mkdir $TOP/buildoutput/$DIR/attr
    cd $TOP/buildoutput/$DIR/attr
    export ATTR_SOURCE=$TOP/packages/attr
    $TOP/Config/attr_tree
    ./configure --prefix= --with-pic --enable-lib64=yes \
          --enable-gettext=no --enable-static=no 
    make
    make DIST_ROOT=$PREFIX install install-dev install-lib
    libtool_fixup $PREFIX/libexec64/libattr.la
    make clean
    export ATTR_SOURCE=
    ;;

    autoconf)
    rm -rf $TOP/buildoutput/$DIR/autoconf
    mkdir $TOP/buildoutput/$DIR/autoconf
    cd $TOP/buildoutput/$DIR/autoconf
    ../../../packages/autoconf/configure --prefix=/usr 
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    bash)
    rm -rf $TOP/buildoutput/$DIR/bash
    mkdir $TOP/buildoutput/$DIR/bash
    cd $TOP/buildoutput/$DIR/bash
    ../../../packages/bash/configure --prefix= \
        --enable-readline --disable-nls 
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    bison)
    rm -rf $TOP/buildoutput/$DIR/bison
    mkdir $TOP/buildoutput/$DIR/bison
    cd $TOP/buildoutput/$DIR/bison
    ../../../packages/bison/configure --prefix=/usr \
        --enable-silent-rules
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    bzip2)
    cd $TOP/packages/bzip2
    make PREFIX=$PREFIX install
    make distclean
    ;;

    coreutils)
    rm -rf $TOP/buildoutput/$DIR/coreutils
    mkdir $TOP/buildoutput/$DIR/coreutils
    cd $TOP/buildoutput/$DIR/coreutils
    ../../../packages/coreutils/configure --prefix= \
        --enable-threads=posix       
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    cracklib)
    rm -rf $TOP/buildoutput/$DIR/cracklib
    mkdir $TOP/buildoutput/$DIR/cracklib
    cd $TOP/buildoutput/$DIR/cracklib
    ../../../packages/cracklib/configure --prefix=/usr --with-pic \
        --with-default-dict=/lib/cracklib/pw_dict
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/usr/lib/libcrack.la
    make distclean
    ;;

    curl)
    rm -rf $TOP/buildoutput/$DIR/curl
    mkdir $TOP/buildoutput/$DIR/curl
    cd $TOP/buildoutput/$DIR/curl
    ../../../packages/curl/configure --prefix=/usr \
        --with-pic LIBS="-lssl -lcrypto" --disable-manual \
        --disable-debug
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/usr/lib/libcurl.la
    make distclean
    ;;

    db)
    rm -rf $TOP/buildoutput/$DIR/db
    mkdir $TOP/buildoutput/$DIR/db
    cd $TOP/buildoutput/$DIR/db
    ../../../packages/db/dist/configure --enable-smallbuild \
     --prefix=/usr --with-pic --enable-dbm --enable-compat185 \
     --without-docs LIBS=-lpthread 
    make
    make DESTDIR=$PREFIX install
    make DESTDIR=$PREFIX uninstall_docs
    libtool_fixup $PREFIX/usr/lib/libdb-5.3.la
    make distclean
    ;;

    dbus)
    rm -rf $TOP/buildoutput/$DIR/dbus
    mkdir $TOP/buildoutput/$DIR/dbus
    cd $TOP/buildoutput/$DIR/dbus
    ../../../packages/dbus/configure --prefix= --with-pic \
        --enable-silent-rules
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libdbus-1.la
    make distclean
    ;;

    debconf)
    cd $TOP/packages/debconf
    make all
    make prefix=$PREFIX install
    make clean
    ;;

    dhcpcd)
    cd $TOP/packages/dhcpcd
    ./configure --prefix=
    make
    make install
    make distclean
    ;;

    diffutils)
    rm -rf $TOP/buildoutput/$DIR/diffutils
    mkdir $TOP/buildoutput/$DIR/diffutils
    cd $TOP/buildoutput/$DIR/diffutils
    ../../../packages/diffutils/configure --prefix= \
        --enable-silent-rules
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    dpkg)
    rm -rf $TOP/buildoutput/$DIR/dpkg
    mkdir $TOP/buildoutput/$DIR/dpkg
    cd $TOP/buildoutput/$DIR/dpkg
    ../../../packages/dpkg/configure --prefix= \
        CURSES_LIBS="-ltinfo"
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    e2fsprogs)
    rm -rf $TOP/buildoutput/$DIR/e2fsprogs
    mkdir $TOP/buildoutput/$DIR/e2fsprogs
    cd $TOP/buildoutput/$DIR/e2fsprogs
    ../../../packages/e2fsprogs/configure --prefix=
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    eject)
    rm -rf $TOP/buildoutput/$DIR/eject
    mkdir $TOP/buildoutput/$DIR/eject
    cd $TOP/buildoutput/$DIR/eject
    export EJECT_SOURCE=$TOP/packages/eject
    $TOP/Config/eject_tree
    ../../../packages/eject/configure --prefix=
    make
    make DESTDIR=$PREFIX install
    make distclean
    export EJECT_SOURCE=
    ;;

    expat)
    rm -rf $TOP/buildoutput/$DIR/expat
    mkdir $TOP/buildoutput/$DIR/expat
    cd $TOP/buildoutput/$DIR/expat
    ../../../packages/expat/configure --prefix= --with-pic 
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libexpat.la
    make distclean
    ;;

    expect)
    rm -rf $TOP/buildoutput/$DIR/expect
    mkdir $TOP/buildoutput/$DIR/expect
    cd $TOP/buildoutput/$DIR/expect
    ../../../packages/expect/configure \
        --prefix= --with-pic \
        --with-tcl=$PREFIX/lib
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    file)
    rm -rf $TOP/buildoutput/$DIR/file
    mkdir $TOP/buildoutput/$DIR/file
    cd $TOP/buildoutput/$DIR/file
    ../../../packages/file/configure --prefix=
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libmagic.la
    make distclean
    ;;

    findutils)
    rm -rf $TOP/buildoutput/$DIR/findutils
    mkdir $TOP/buildoutput/$DIR/findutils
    cd $TOP/buildoutput/$DIR/findutils
    ../../../packages/findutils/configure --prefix=
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    flex)
    rm -rf $TOP/buildoutput/$DIR/flex
    mkdir $TOP/buildoutput/$DIR/flex
    cd $TOP/buildoutput/$DIR/flex
    ../../../packages/flex/configure --prefix=
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    gawk)
    rm -rf $TOP/buildoutput/$DIR/gawk
    mkdir $TOP/buildoutput/$DIR/gawk
    cd $TOP/buildoutput/$DIR/gawk
    ../../../packages/gawk/configure --prefix=
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    glib)
    rm -rf $TOP/buildoutput/$DIR/glib
    mkdir $TOP/buildoutput/$DIR/glib
    cd $TOP/buildoutput/$DIR/glib
    ../../../packages/glib/configure --prefix= \
        --with-pic --disable-man \
    DBUS1_CFLAGS="-I$PREFIX/include/dbus-1.0 -I$PREFIX/lib/dbus-1.0/include" \
    LIBFFI_CFLAGS="-I$PREFIX/lib/libffi-3.0.11/include"
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libgio-2.0.la
    libtool_fixup $PREFIX/lib/libglib-2.0.la
    libtool_fixup $PREFIX/lib/libgmodule-2.0.la
    libtool_fixup $PREFIX/lib/libgthread-2.0.la
    libtool_fixup $PREFIX/lib/libgobject-2.0.la
    make distclean
    ;;

    eglibc)
    if [ $NOS = 1 ] || [ $LXDE = 1 ]
    then
    rm -rf $TOP/buildoutput/$DIR/eglibc
    mkdir $TOP/buildoutput/$DIR/eglibc
    cd $TOP/buildoutput/$DIR/eglibc
    ../../../packages/eglibc/configure --prefix=/usr \
        --enable-kernel=2.6.25 --disable-profile \
        --libexecdir=/usr/lib/glibc \
        --with-headers=$TOP/linux-headers/include \
        CFLAGS="-fPIC -Os" libc_cv_pic_default=yes 
    make
    make install_root=$PREFIX install
    cp $TOP/packages/glibc/sunrpc/rpc/*.h $PREFIX/usr/include/rpc
    cp $TOP/packages/glibc/sunrpc/rpcsvc/*.h $PREFIX/usr/include/rpcsvc
    cp $TOP/packages/glibc/nis/rpcsvc/*.h $PREFIX/usr/include/rpcsvc
    cp $TOP/boot/etc/nsswitch.conf $PREFIX/etc
    cp $TOP/boot/localtime $PREFIX/etc
    mkdir $PREFIX/usr/lib/locale
    install_locale $PREFIX/usr/lib/locale
    mkdir $PREFIX/usr/share/zoneinfo
    install_timezone $PREFIX/usr/share/zoneinfo
    make distclean
    fi
    ;;
    
    glibc)
    if [ $LINUX = 1 ]
    then
    rm -rf $TOP/buildoutput/$DIR/glibc
    mkdir $TOP/buildoutput/$DIR/glibc
    cd $TOP/buildoutput/$DIR/glibc
    ../../../packages/glibc/configure --prefix=/usr \
        --enable-kernel=2.6.25 --disable-profile \
        --libexecdir=/usr/lib/glibc --with-elf \
        --with-headers=$TOP/linux-headers/include \
        CFLAGS="-fPIC -Os" libc_cv_pic_default=yes 
    make
    make install_root=$PREFIX install
    cp $TOP/packages/glibc/sunrpc/rpc/*.h $PREFIX/usr/include/rpc
    cp $TOP/packages/glibc/sunrpc/rpcsvc/*.h $PREFIX/usr/include/rpcsvc
    cp $TOP/packages/glibc/nis/rpcsvc/*.h $PREFIX/usr/include/rpcsvc
    cp $TOP/boot/etc/nsswitch.conf $PREFIX/etc
    cp $TOP/boot/localtime $PREFIX/etc
    mkdir $PREFIX/usr/lib/locale
    install_locale $PREFIX/usr/lib/locale
    mkdir $PREFIX/usr/share/zoneinfo
    install_timezone $PREFIX/usr/share/zoneinfo
    make distclean
    fi

    gmp)
    rm -rf $TOP/buildoutput/$DIR/gmp
    mkdir $TOP/buildoutput/$DIR/gmp
    cd $TOP/buildoutput/$DIR/gmp
    ../../../packages/gmp/configure --prefix= --with-pic \
        --enable-cxx --enable-mpbsd 
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libgmp.la
    libtool_fixup $PREFIX/lib/libgmpxx.la
    libtool_fixup $PREFIX/lib/libmp.la
    make distclean
    ;;

    gnutls)
    rm -rf $TOP/buildoutput/$DIR/gnutls
    mkdir $TOP/buildoutput/$DIR/gnutls
    cd $TOP/buildoutput/$DIR/gnutls
    export GNUTLS_SOURCE=$TOP/packages/gnutls
    $TOP/Config/gnutls_tree
    ./configure --prefix= --enable-silent-rules \
           --disable-libdane --enable-gtk-doc-html=no
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libgnutls.la
    libtool_fixup $PREFIX/lib/libgnutls-openssl.la
    libtool_fixup $PREFIX/lib/libgnutlsxx.la
    make distclean
    export GNUTLS_SOURCE=
    ;;

    grep)
    rm -rf $TOP/buildoutput/$DIR/grep
    mkdir $TOP/buildoutput/$DIR/grep
    cd $TOP/buildoutput/$DIR/grep
    ../../../packages/grep/configure --prefix=
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    gzip)
    rm -rf $TOP/buildoutput/$DIR/gzip
    mkdir $TOP/buildoutput/$DIR/gzip
    cd $TOP/buildoutput/$DIR/gzip
    ../../../packages/gzip/configure --prefix=
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    iana-etc)
    cd $TOP/packages/iana-etc
    make DESTDIR=$PREFIX install
    make clean
    export DESTDIR=
    ;;

    inetutils)
    rm -rf $TOP/buildoutput/$DIR/inetutils
    mkdir $TOP/buildoutput/$DIR/inetutils
    cd $TOP/buildoutput/$DIR/inetutils
    ../../../packages/inetutils/configure --prefix= \
       --disable-ifconfig --disable-logger --disable-syslogd \
       --disable-servers --disable-ping --disable-talk \
       --disable-ping6 --disable-hostname --enable-silent-rules
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    js)
    rm -rf $TOP/buildoutput/$DIR/js
    mkdir $TOP/buildoutput/$DIR/js
    cd $TOP/buildoutput/$DIR/js
    ../../../packages/js/js/src/configure --prefix= \
        --disable-tests
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    kbd)
    rm -rf $TOP/buildoutput/$DIR/kbd
    mkdir $TOP/buildoutput/$DIR/kbd
    cd $TOP/buildoutput/$DIR/kbd
    ../../../packages/kbd/configure --prefix= \
        --disable-vlock
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    kmod)
    rm -rf $TOP/buildoutput/$DIR/kmod
    mkdir $TOP/buildoutput/$DIR/kmod
    cd $TOP/buildoutput/$DIR/kmod
    ../../../packages/kmod/configure --prefix= \
        --with-pic --disable-manpages --with-xz --with-zlib
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libkmod.la
    make distclean
    ;;

    libarchive)
    rm -rf $TOP/buildoutput/$DIR/libarchive
    mkdir $TOP/buildoutput/$DIR/libarchive
    cd $TOP/buildoutput/$DIR/libarchive
    ../../../packages/libarchive/configure \
        --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    libcap)
    cd $TOP/packages/libcap
    make DESTDIR=$PREFIX RAISE_SETFCAP=no install
    make distclean
    export DESTDIR=
    ;;

    libffi)
    rm -rf $TOP/buildoutput/$DIR/libffi
    mkdir $TOP/buildoutput/$DIR/libffi
    cd $TOP/buildoutput/$DIR/libffi
    ../../../packages/libffi/configure \
        --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libffi.la
    make distclean
    ;;

    libgcrypt)
    rm -rf $TOP/buildoutput/$DIR/libgcrypt
    mkdir $TOP/buildoutput/$DIR/libgcrypt
    cd $TOP/buildoutput/$DIR/libgcrypt
    ../../../packages/libgcrypt/configure \
        --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libgcrypt.la
    make distclean
    ;;

    libgpg-error)
    rm -rf $TOP/buildoutput/$DIR/libgpg-error
    mkdir $TOP/buildoutput/$DIR/libgpg-error
    cd $TOP/buildoutput/$DIR/libgpg-error
    ../../../packages/libgpg-error/configure \
        --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libgpg-error.la
    make distclean
    ;;

    libnfnetlink)
    rm -rf $TOP/buildoutput/$DIR/libnfnetlink
    mkdir $TOP/buildoutput/$DIR/libnfnetlink
    cd $TOP/buildoutput/$DIR/libnfnetlink
    ../../../packages/libnfnetlink/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libnfnetlink.la
    make distclean
    ;;

    libnl)
    rm -rf $TOP/buildoutput/$DIR/libnl
    mkdir $TOP/buildoutput/$DIR/libnl
    cd $TOP/buildoutput/$DIR/libnl
    ../../../packages/libnl/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libnl-3.la
    libtool_fixup $PREFIX/lib/libnl-cli-3.la
    libtool_fixup $PREFIX/lib/libnl-genl-3.la
    libtool_fixup $PREFIX/lib/libnl-nf-3.la
    libtool_fixup $PREFIX/lib/libnl-route-3.la
    libtool_fixup $PREFIX/lib/libnl/cli/qdisc/plug.la
    libtool_fixup $PREFIX/lib/libnl/cli/qdisc/pfifo.la
    libtool_fixup $PREFIX/lib/libnl/cli/qdisc/htb.la
    libtool_fixup $PREFIX/lib/libnl/cli/qdisc/bfifo.la
    libtool_fixup $PREFIX/lib/libnl/cli/qdisc/blackhole.la
    libtool_fixup $PREFIX/lib/libnl/cli/cls/basic.la
    libtool_fixup $PREFIX/lib/libnl/cli/cls/cgroup.la
    make distclean
    ;;

    libpcap)
    rm -rf $TOP/buildoutput/$DIR/libpcap
    mkdir $TOP/buildoutput/$DIR/libpcap
    cd $TOP/buildoutput/$DIR/libpcap
    ../../../packages/libpcap/configure \
        --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    libpipeline)
    rm -rf $TOP/buildoutput/$DIR/libpipeline
    mkdir $TOP/buildoutput/$DIR/libpipeline
    cd $TOP/buildoutput/$DIR/libpipeline
    ../../../packages/libpipeline/configure \
        --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libpipeline.la
    make distclean
    ;;

    libusb)
    rm -rf $TOP/buildoutput/$DIR/libusb
    mkdir $TOP/buildoutput/$DIR/libusb
    cd $TOP/buildoutput/$DIR/libusb
    ../../../packages/libusb/configure --prefix= \
        --with-pic --enable-shared
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    libxml2)
    cd $TOP/packages/libxml2
    ./configure --prefix= \
        --with-pic --enable-shared --with-python=$PREFIX
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libxml2.la
    make distclean
    ;;

    libxslt)
    cd $TOP/packages/libxslt
    XSLTPROC=$TOP/packages/libxslt/xsltproc/xsltproc \
    ./configure --prefix= \
        --with-pic --enable-shared --with-python=$PREFIX \
        --with-libxml-include-prefix=$PREFIX/include/libxml2
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libxslt.la
    libtool_fixup $PREFIX/lib/libexslt.la
    make distclean
    ;;

    make)
    rm -rf $TOP/buildoutput/$DIR/make
    mkdir $TOP/buildoutput/$DIR/make
    cd $TOP/buildoutput/$DIR/make
    ../../../packages/make/configure --prefix=/usr
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    m4)
    rm -rf $TOP/buildoutput/$DIR/m4
    mkdir $TOP/buildoutput/$DIR/m4
    cd $TOP/buildoutput/$DIR/m4
    ../../../packages/m4/configure --prefix=/usr
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    man-db)
    rm -rf $TOP/buildoutput/$DIR/man-db
    mkdir $TOP/buildoutput/$DIR/man-db
    cd $TOP/buildoutput/$DIR/man-db
    ../../../packages/man-db/configure \
            --prefix= --disable-setuid
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/man-db/libman.la
    libtool_fixup $PREFIX/lib/man-db/libmandb.la
    make distclean
    ;;

    man-pages)
    cd $TOP/packages/man-pages
    make DESTDIR=$PREFIX install
    export DESTDIR=
    ;;

    mpc)
    rm -rf $TOP/buildoutput/$DIR/mpc
    mkdir $TOP/buildoutput/$DIR/mpc
    cd $TOP/buildoutput/$DIR/mpc
    ../../../packages/mpc/configure --prefix= \
        --with-gmp=$PREFIX 
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libmpc.la
    libtool_fixup $PREFIX/lib/libmp.la
    make distclean
    ;;

    mpfr)
    rm -rf $TOP/buildoutput/$DIR/mpfr
    mkdir $TOP/buildoutput/$DIR/mpfr
    cd $TOP/buildoutput/$DIR/mpfr
    ../../../packages/mpfr/configure --prefix= \
        --with-gmp=$PREFIX --enable-thread-safe 
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libmpfr.la
    make distclean
    ;;

    ncurses)
    rm -rf $TOP/buildoutput/$DIR/ncurses
    mkdir $TOP/buildoutput/$DIR/ncurses
    cd $TOP/buildoutput/$DIR/ncurses
    ../../../packages/ncurses/configure --prefix= \
        --with-shared --enable-widec --without-tests \
        --without-debug --without-manpages --with-termlib
    make
    make DESTDIR=$PREFIX install
    make distclean

    rm -rf $TOP/buildoutput/$DIR/ncurses
    mkdir $TOP/buildoutput/$DIR/ncurses
    cd $TOP/buildoutput/$DIR/ncurses
    ../../../packages/ncurses/configure --prefix= \
        --with-shared --without-debug --without-tests \
        --without-manpages --with-termlib
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    nettle)
    rm -rf $TOP/buildoutput/$DIR/nettle
    mkdir $TOP/buildoutput/$DIR/nettle
    cd $TOP/buildoutput/$DIR/nettle
    ../../../packages/nettle/configure \
            --prefix=
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    nspr)
    rm -rf $TOP/buildoutput/$DIR/nspr
    mkdir $TOP/buildoutput/$DIR/nspr
    cd $TOP/buildoutput/$DIR/nspr
    ../../../packages/nspr/mozilla/nsprpub/configure \
        --prefix= --enable-pic --with-mozilla --enable-64bit 
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    openssl)
    rm -rf $TOP/buildoutput/$DIR/openssl
    mkdir $TOP/buildoutput/$DIR/openssl
    cd $TOP/buildoutput/$DIR/openssl
    export OPENSSL_SOURCE=$TOP/packages/openssl
    $TOP/Config/openssl_tree
    ./Configure --prefix=/usr linux-x86_64 \
            -L$PREFIX/lib shared -I$PREFIX/include -fPIC
    make
    make INSTALL_PREFIX=$PREFIX install
    make clean
    ;;

    pam)
    rm -rf $TOP/buildoutput/$DIR/pam
    mkdir $TOP/buildoutput/$DIR/pam
    cd $TOP/buildoutput/$DIR/pam
    ../../../packages/pam/configure --prefix= \
        --with-pic --includedir=/include/security
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libpamc.la
    libtool_fixup $PREFIX/lib/libpam.la
    libtool_fixup $PREFIX/lib/libpam_misc.la
    libtool_fixup $PREFIX/lib/security/pam_localuser.la
    libtool_fixup $PREFIX/lib/security/pam_echo.la
    libtool_fixup $PREFIX/lib/security/pam_umask.la
    libtool_fixup $PREFIX/lib/security/pam_group.la
    libtool_fixup $PREFIX/lib/security/pam_limits.la
    libtool_fixup $PREFIX/lib/security/pam_faildelay.la
    libtool_fixup $PREFIX/lib/security/pam_mkhomedir.la
    libtool_fixup $PREFIX/lib/security/pam_unix.la
    libtool_fixup $PREFIX/lib/security/pam_motd.la
    libtool_fixup $PREFIX/lib/security/pam_stress.la
    libtool_fixup $PREFIX/lib/security/pam_succeed_if.la
    libtool_fixup $PREFIX/lib/security/pam_time.la
    libtool_fixup $PREFIX/lib/security/pam_securetty.la
    libtool_fixup $PREFIX/lib/security/pam_issue.la
    libtool_fixup $PREFIX/lib/security/pam_debug.la
    libtool_fixup $PREFIX/lib/security/pam_keyinit.la
    libtool_fixup $PREFIX/lib/security/pam_access.la
    libtool_fixup $PREFIX/lib/security/pam_timestamp.la
    libtool_fixup $PREFIX/lib/security/pam_tally2.la
    libtool_fixup $PREFIX/lib/security/pam_wheel.la
    libtool_fixup $PREFIX/lib/security/pam_lastlog.la
    libtool_fixup $PREFIX/lib/security/pam_mail.la
    libtool_fixup $PREFIX/lib/security/pam_loginuid.la
    libtool_fixup $PREFIX/lib/security/pam_nologin.la
    libtool_fixup $PREFIX/lib/security/pam_exec.la
    libtool_fixup $PREFIX/lib/security/pam_listfile.la
    libtool_fixup $PREFIX/lib/security/pam_cracklib.la
    libtool_fixup $PREFIX/lib/security/pam_permit.la
    libtool_fixup $PREFIX/lib/security/pam_deny.la
    libtool_fixup $PREFIX/lib/security/pam_namespace.la
    libtool_fixup $PREFIX/lib/security/pam_rootok.la
    libtool_fixup $PREFIX/lib/security/pam_warn.la
    libtool_fixup $PREFIX/lib/security/pam_tally.la
    libtool_fixup $PREFIX/lib/security/pam_xauth.la
    libtool_fixup $PREFIX/lib/security/pam_shells.la
    libtool_fixup $PREFIX/lib/security/pam_userdb.la
    libtool_fixup $PREFIX/lib/security/pam_rhosts.la
    libtool_fixup $PREFIX/lib/security/pam_env.la
    libtool_fixup $PREFIX/lib/security/pam_filter.la
    libtool_fixup $PREFIX/lib/security/pam_pwhistory.la
    libtool_fixup $PREFIX/lib/security/pam_ftp.la
    make distclean
    ;;

    pcre)
    rm -rf $TOP/buildoutput/$DIR/pcre
    mkdir $TOP/buildoutput/$DIR/pcre
    cd $TOP/buildoutput/$DIR/pcre
    ../../../packages/pcre/configure --prefix= 
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libpcre.la
    libtool_fixup $PREFIX/lib/libpcrecpp.la
    libtool_fixup $PREFIX/lib/libpcreposix.la
    make distclean
    ;;

    php)
    rm -rf $TOP/buildoutput/$DIR/php
    mkdir $TOP/buildoutput/$DIR/php
    cd $TOP/buildoutput/$DIR/php
    ../../../packages/php/configure \
        --prefix= --with-pic --with-libxml-dir=$PREFIX \
        CFLAGS="$CFLAGS -I$PREFIX/include/libxml2"
    make
    make INSTALL_ROOT=$PREFIX install
    make distclean
    ;;

    popt)
    rm -rf $TOP/buildoutput/$DIR/popt
    mkdir $TOP/buildoutput/$DIR/popt
    cd $TOP/buildoutput/$DIR/popt
    ../../../packages/popt/configure \
            --prefix= --with-pic 
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libpopt.la
    make distclean
    ;;

    procps)
    cd $TOP/packages/procps
    make DESTDIR=$PREFIX install="install -D" \
        ldconfig=echo install
    make clean
    export DESTDIR=
    ;;

    readline)
    rm -rf $TOP/buildoutput/$DIR/readline
    mkdir $TOP/buildoutput/$DIR/readline
    cd $TOP/buildoutput/$DIR/readline
    ../../../packages/readline/configure \
        --prefix= --with-curses
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    sed)
    rm -rf $TOP/buildoutput/$DIR/sed
    mkdir $TOP/buildoutput/$DIR/sed
    cd $TOP/buildoutput/$DIR/sed
    ../../../packages/sed/configure --prefix= \
                --with-pic 
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    shadow)
    rm -rf $TOP/buildoutput/$DIR/shadow
    mkdir $TOP/buildoutput/$DIR/shadow
    cd $TOP/buildoutput/$DIR/shadow
    ../../../packages/shadow/configure --prefix= \
                --with-pic --with-libcrack
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    sysvinit)
    cd $TOP/packages/sysvinit
    make -C src 
    make -C src ROOT=$PREFIX install
    mv $PREFIX/sbin/init $PREFIX/sbin/init.sysv
    make clean
    ;;

    systemd)
    rm -rf $TOP/buildoutput/$DIR/systemd
    mkdir $TOP/buildoutput/$DIR/systemd
    cd $TOP/buildoutput/$DIR/systemd
    ../../../packages/systemd/configure --disable-nls \
    --prefix= --with-pic --disable-manpages \
    --with-rootlibdir=/lib --without-python \
    --with-bashcompletiondir=/etc --with-pamlibdir=/lib \
    --with-sysvinit-path=/etc/rc.d/init.d \
    --with-sysvrcnd-path=/etc/rc.d
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libgudev-1.0.la
    libtool_fixup $PREFIX/lib/libudev.la
    libtool_fixup $PREFIX/lib/libsystemd-login.la
    libtool_fixup $PREFIX/lib/libsystemd-journal.la
    libtool_fixup $PREFIX/lib/libsystemd-id128.la
    libtool_fixup $PREFIX/lib/libsystemd-daemon.la
    make distclean
    ;;

    tar)
    rm -rf $TOP/buildoutput/$DIR/tar
    mkdir $TOP/buildoutput/$DIR/tar
    cd $TOP/buildoutput/$DIR/tar
    ../../../packages/tar/configure --prefix=
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    tcl)
    rm -rf $TOP/buildoutput/$DIR/tcl
    mkdir $TOP/buildoutput/$DIR/tcl
    cd $TOP/buildoutput/$DIR/tcl
    ../../../packages/tcl/unix/configure --prefix=
    make
    make DESTDIR=$PREFIX install
    make DESTDIR=$PREFIX install-private-headers
    make distclean
    cd ..
    ;;

    texinfo)
    rm -rf $TOP/buildoutput/$DIR/texinfo
    mkdir $TOP/buildoutput/$DIR/texinfo
    cd $TOP/buildoutput/$DIR/texinfo
    ../../../packages/texinfo/configure --prefix=/usr
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    util-linux)
    rm -rf $TOP/buildoutput/$DIR/util-linux
    mkdir $TOP/buildoutput/$DIR/util-linux
    cd $TOP/buildoutput/$DIR/util-linux
    ../../../packages/util-linux/configure --enable-silent-rules \
        --disable-makeinstall-setuid --disable-makeinstall-chown \
        --prefix= --with-pic --disable-su --disable-sulogin \
        --disable-login --disable-runuser --without-ncurses 
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libuuid.la
    libtool_fixup $PREFIX/lib/libblkid.la
    libtool_fixup $PREFIX/lib/libmount.la
    make distclean
    ;;

    vim)
    cd $TOP/packages/vim
    ./configure --prefix= --with-tlib=tinfo --enable-multibyte \
        --with-features=tiny --disable-gtktest
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    wireless_tools)
    cd $TOP/packages/wireless_tools
    make
    make PREFIX=$PREFIX install
    make clean
    ;;

    wpa_supplicant)
    cd $TOP/packages/wpa_supplicant/wpa_supplicant
    cp .config_full .config
    make
    make DESTDIR=$PREFIX install
    make clean
    export DESTDIR=
    ;;

    xz)
    rm -rf $TOP/buildoutput/$DIR/xz
    mkdir $TOP/buildoutput/$DIR/xz
    cd $TOP/buildoutput/$DIR/xz
    ../../../packages/xz/configure --prefix= --with-pic 
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/liblzma.la
    make distclean
    ;;

    zlib)
    rm -rf $TOP/buildoutput/$DIR/zlib
    mkdir $TOP/buildoutput/$DIR/zlib
    cd $TOP/buildoutput/$DIR/zlib
    export ZLIB_SOURCE=$TOP/packages/zlib
    $TOP/Config/zlib_tree
    ./configure --prefix= --libdir=/lib
    make
    make DESTDIR=$PREFIX install
    make distclean
    export ZLIB_SOURCE=
    ;;
 
    zip)
    cd $TOP/packages/zip
    make -f unix/Makefile prefix=$PREFIX generic install
    make -f unix/Makefile clean
    export prefix=
    ;;

    *)
    echo "Wrong $DIR pack"
    exit 1
    ;;
    esac
}


function build_lxde_component {

    echo "----------------------------------------------------------"
    echo "------------------- building $1 --------------------------"
    echo "----------------------------------------------------------"

    case "$1" in
    lxappearance)
    rm -rf $TOP/buildoutput/lxde/lxappearance
    mkdir $TOP/buildoutput/lxde/lxappearance
    cd $TOP/buildoutput/lxde/lxappearance
    ../../../lxde/lxappearance/configure --prefix= \
        LIBS=-lX11 --enable-man --enable-dbus --sysconfdir=/etc
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxconf)
    rm -rf $TOP/buildoutput/lxde/lxconf
    mkdir $TOP/buildoutput/lxde/lxconf
    cd $TOP/buildoutput/lxde/lxconf
    ../../../lxde/lxconf/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxde-common)
    rm -rf $TOP/buildoutput/lxde/lxde-common
    mkdir $TOP/buildoutput/lxde/lxde-common
    cd $TOP/buildoutput/lxde/lxde-common
    ../../../lxde/lxde-common/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxde-icon-theme)
    rm -rf $TOP/buildoutput/lxde/lxde-icon-theme
    mkdir $TOP/buildoutput/lxde/lxde-icon-theme
    cd $TOP/buildoutput/lxde/lxde-icon-theme
    ../../../lxde/lxde-icon-theme/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxdm)
    rm -rf $TOP/buildoutput/lxde/lxdm
    mkdir $TOP/buildoutput/lxde/lxdm
    cd $TOP/buildoutput/lxde/lxdm
    ../../../lxde/lxdm/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxinput)
    rm -rf $TOP/buildoutput/lxde/lxinput
    mkdir $TOP/buildoutput/lxde/lxinput
    cd $TOP/buildoutput/lxde/lxinput
    ../../../lxde/lxinput/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxlauncher)
    rm -rf $TOP/buildoutput/lxde/lxlauncher
    mkdir $TOP/buildoutput/lxde/lxlauncher
    cd $TOP/buildoutput/lxde/lxlauncher
    ../../../lxde/lxlauncher/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxmenu-data)
    rm -rf $TOP/buildoutput/lxde/lxmenu-data
    mkdir $TOP/buildoutput/lxde/lxmenu-data
    cd $TOP/buildoutput/lxde/lxmenu-data
    ../../../lxde/lxmenu-data/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxpanel)
    rm -rf $TOP/buildoutput/lxde/lxpanel
    mkdir $TOP/buildoutput/lxde/lxpanel
    cd $TOP/buildoutput/lxde/lxpanel
    ../../../lxde/lxpanel/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxpanel-plugins)
    rm -rf $TOP/buildoutput/lxde/lxpanel-plugins
    mkdir $TOP/buildoutput/lxde/lxpanel-plugins

    mkdir $TOP/buildoutput/lxde/lxpanel-plugins/menu_2
    cd $TOP/buildoutput/lxde/lxpanel-plugins/menu_2
    ../../../../lxde/lxpanel-plugins/menu_2/configure --prefix= \
        --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean

    mkdir $TOP/buildoutput/lxde/lxpanel-plugins/batt
    cd $TOP/buildoutput/lxde/lxpanel-plugins/batt
    ../../../../lxde/lxpanel-plugins/batt/configure --prefix= \
        --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean

    mkdir $TOP/buildoutput/lxde/lxpanel-plugins/drives
    cd $TOP/buildoutput/lxde/lxpanel-plugins/drives
    ../../../../lxde/lxpanel-plugins/drives/configure --prefix= \
        --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean

    mkdir $TOP/buildoutput/lxde/lxpanel-plugins/netstat
    cd $TOP/buildoutput/lxde/lxpanel-plugins/netstat
    ../../../../lxde/lxpanel-plugins/netstat/configure --prefix= \
        --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean

    mkdir $TOP/buildoutput/lxde/lxpanel-plugins/xkb
    cd $TOP/buildoutput/lxde/lxpanel-plugins/xkb
    ../../../../lxde/lxpanel-plugins/xkb/configure --prefix= \
        --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxrandr)
    rm -rf $TOP/buildoutput/lxde/lxrandr
    mkdir $TOP/buildoutput/lxde/lxrandr
    cd $TOP/buildoutput/lxde/lxrandr
    ../../../lxde/lxrandr/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxsession)
    rm -rf $TOP/buildoutput/lxde/lxsession
    mkdir $TOP/buildoutput/lxde/lxsession
    cd $TOP/buildoutput/lxde/lxsession
    ../../../lxde/lxsession/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxsession-edit)
    rm -rf $TOP/buildoutput/lxde/lxsession-edit
    mkdir $TOP/buildoutput/lxde/lxsession-edit
    cd $TOP/buildoutput/lxde/lxsession-edit
    ../../../lxde/lxsession-edit/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxshortcut)
    rm -rf $TOP/buildoutput/lxde/lxshortcut
    mkdir $TOP/buildoutput/lxde/lxshortcut
    cd $TOP/buildoutput/lxde/lxshortcut
    ../../../lxde/lxshortcut/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxtask)
    rm -rf $TOP/buildoutput/lxde/lxtask
    mkdir $TOP/buildoutput/lxde/lxtask
    cd $TOP/buildoutput/lxde/lxtask
    ../../../lxde/lxtask/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    lxterminal)
    rm -rf $TOP/buildoutput/lxde/lxterminal
    mkdir $TOP/buildoutput/lxde/lxterminal
    cd $TOP/buildoutput/lxde/lxterminal
    ../../../lxde/lxterminal/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    menu-cache)
    rm -rf $TOP/buildoutput/lxde/menu-cache
    mkdir $TOP/buildoutput/lxde/menu-cache
    cd $TOP/buildoutput/lxde/menu-cache
    ../../../lxde/menu-cache/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    *)
    echo "Wrong lxde pack"
    exit 1
    ;;
    esac
}

function buildTPD_display_component {

    echo "----------------------------------------------------------"
    echo "------------------- building $1 --------------------------"
    echo "----------------------------------------------------------"

    DIR=$2

    case $1 in
    accountsservice)
        rm -rf $TOP/buildoutput/$DIR/accountsservice
        mkdir $TOP/buildoutput/$DIR/accountsservice
        cd $TOP/buildoutput/$DIR/accountsservice
        ../../../tpd/display/accountsservice/configure --prefix= \
            --with-pic --enable-introspection=no
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    atk)
        rm -rf $TOP/buildoutput/$DIR/atk
        mkdir $TOP/buildoutput/$DIR/atk
        cd $TOP/buildoutput/$DIR/atk
        ../../../tpd/display/atk/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

     at-spi)
        rm -rf $TOP/buildoutput/$DIR/at-spi
        mkdir $TOP/buildoutput/$DIR/at-spi
        cd $TOP/buildoutput/$DIR/at-spi
        ../../../tpd/display/at-spi/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    at-spi2-atk)
        rm -rf $TOP/buildoutput/$DIR/at-spi2-atk
        mkdir $TOP/buildoutput/$DIR/at-spi2-atk
        cd $TOP/buildoutput/$DIR/at-spi2-atk
        ../../../tpd/display/at-spi2-atk/configure --prefix= -with-pic
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    at-spi2-core)
        rm -rf $TOP/buildoutput/$DIR/at-spi2-core
        mkdir $TOP/buildoutput/$DIR/at-spi2-core
        cd $TOP/buildoutput/$DIR/at-spi2-core
        ../../../tpd/display/at-spi2-core/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    bigreqsproto)
        rm -rf $TOP/buildoutput/$DIR/bigreqsproto
        mkdir $TOP/buildoutput/$DIR/bigreqsproto
        cd $TOP/buildoutput/$DIR/bigreqsproto
        ../../../tpd/display/bigreqsproto/configure --prefix= \
            --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    cairo)
        rm -rf $TOP/buildoutput/$DIR/cairo
        mkdir $TOP/buildoutput/$DIR/cairo
        cd $TOP/buildoutput/$DIR/cairo
        ../../../tpd/display/cairo/configure --prefix= \
            --with-pic --enable-xml 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    clutter)
        rm -rf $TOP/buildoutput/$DIR/clutter
        mkdir $TOP/buildoutput/$DIR/clutter
        cd $TOP/buildoutput/$DIR/clutter
        ../../../tpd/display/clutter/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    colord)
        rm -rf $TOP/buildoutput/$DIR/colord
        mkdir $TOP/buildoutput/$DIR/colord
        cd $TOP/buildoutput/$DIR/colord
        ../../../tpd/display/colord/configure --prefix= \
            --with-pic --enable-gusb=no
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    compositeproto)
        rm -rf $TOP/buildoutput/$DIR/compositeproto
        mkdir $TOP/buildoutput/$DIR/compositeproto
        cd $TOP/buildoutput/$DIR/compositeproto
        ../../../tpd/display/compositeproto/configure --prefix=  
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    ConsoleKit)
        rm -rf $TOP/buildoutput/$DIR/ConsoleKit
        mkdir $TOP/buildoutput/$DIR/ConsoleKit
        cd $TOP/buildoutput/$DIR/ConsoleKit
        ../../../tpd/display/ConsoleKit/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    damageproto)
        rm -rf $TOP/buildoutput/$DIR/damageproto
        mkdir $TOP/buildoutput/$DIR/damageproto
        cd $TOP/buildoutput/$DIR/damageproto
        ../../../tpd/display/damageproto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    dconf)
        rm -rf $TOP/buildoutput/$DIR/dri2proto
        mkdir $TOP/buildoutput/$DIR/dri2proto
        cd $TOP/buildoutput/$DIR/dri2proto
        ../../../tpd/display/dri2proto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    dri2proto)
        rm -rf $TOP/buildoutput/$DIR/dri2proto
        mkdir $TOP/buildoutput/$DIR/dri2proto
        cd $TOP/buildoutput/$DIR/dri2proto
        ../../../tpd/display/dri2proto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    encodings)
        rm -rf $TOP/buildoutput/$DIR/encodings
        mkdir $TOP/buildoutput/$DIR/encodings
        cd $TOP/buildoutput/$DIR/encodings
        ../../../tpd/display/encodings/configure --prefix= \
            --with-fontrootdir=$PREFIX/usr/share/fonts/X11
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    evolution-data-server)
        rm -rf $TOP/buildoutput/$DIR/evolution-data-server
        mkdir $TOP/buildoutput/$DIR/evolution-data-server
        cd $TOP/buildoutput/$DIR/evolution-data-server
        ../../../tpd/display/evolution-data-server/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    fixesproto)
        rm -rf $TOP/buildoutput/$DIR/fixesproto
        mkdir $TOP/buildoutput/$DIR/fixesproto
        cd $TOP/buildoutput/$DIR/fixesproto
        ../../../tpd/display/fixesproto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    font-adobe-100dpi)
        rm -rf $TOP/buildoutput/$DIR/font-adobe-100dpi
        mkdir $TOP/buildoutput/$DIR/font-adobe-100dpi
        cd $TOP/buildoutput/$DIR/font-adobe-100dpi
        ../../../tpd/display/font-adobe-100dpi/configure --prefix= \
            --enable-silent-rules \
            --with-fontrootdir=$PREFIX/usr/share/fonts/X11
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    font-bh-100dpi)
        rm -rf $TOP/buildoutput/$DIR/font-bh-100dpi
        mkdir $TOP/buildoutput/$DIR/font-bh-100dpi
        cd $TOP/buildoutput/$DIR/font-bh-100dpi
        ../../../tpd/display/font-bh-100dpi/configure --prefix= \
            --enable-silent-rules \
            --with-fontrootdir=$PREFIX/usr/share/fonts/X11
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    fontcacheproto)
        rm -rf $TOP/buildoutput/$DIR/fontcacheproto
        mkdir $TOP/buildoutput/$DIR/fontcacheproto
        cd $TOP/buildoutput/$DIR/fontcacheproto
        ../../../tpd/display/fontcacheproto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    fontconfig)
        rm -rf $TOP/buildoutput/$DIR/fontconfig
        mkdir $TOP/buildoutput/$DIR/fontconfig
        cd $TOP/buildoutput/$DIR/fontconfig
        export FONTCONFIG_SOURCE=$TOP/tpd/display/fontconfig
        $TOP/Config/fontconfig_tree
        ./configure --prefix= --with-pic --disable-docs
        make
        make DESTDIR=$PREFIX install
        make distclean
        export FONTCONFIG_SOURCE=
        ;;

    font-mutt-misc)
        rm -rf $TOP/buildoutput/$DIR/font-mutt-misc
        mkdir $TOP/buildoutput/$DIR/font-mutt-misc
        cd $TOP/buildoutput/$DIR/font-mutt-misc
        ../../../tpd/display/font-mutt-misc/configure --prefix= \
            --enable-silent-rules --with-fontrootdir=$PREFIX/usr/share/fonts/X11
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    fontsproto)
        rm -rf $TOP/buildoutput/$DIR/fontsproto
        mkdir $TOP/buildoutput/$DIR/fontsproto
        cd $TOP/buildoutput/$DIR/fontsproto
        ../../../tpd/display/fontsproto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    font-util)
        rm -rf $TOP/buildoutput/$DIR/font-util
        mkdir $TOP/buildoutput/$DIR/font-util
        cd $TOP/buildoutput/$DIR/font-util
        ../../../tpd/display/font-util/configure --prefix=   
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    fribidi)
        rm -rf $TOP/buildoutput/$DIR/fribidi
        mkdir $TOP/buildoutput/$DIR/fribidi
        cd $TOP/buildoutput/$DIR/fribidi
        ../../../tpd/display/fribidi/configure \
            --prefix= --with-pic
        make
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    freetype)
        rm -rf $TOP/buildoutput/$DIR/freetype
        mkdir $TOP/buildoutput/$DIR/freetype
        cd $TOP/buildoutput/$DIR/freetype
        ../../../tpd/display/freetype/configure \
            --prefix= --with-pic
        make
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    fribidi)
        rm -rf $TOP/buildoutput/$DIR/fribidi
        mkdir $TOP/buildoutput/$DIR/fribidi
        cd $TOP/buildoutput/$DIR/fribidi
        ../../../tpd/display/fribidi/configure \
            --prefix= --with-pic
        make
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    GConf)
        rm -rf $TOP/buildoutput/$DIR/GConf
        mkdir $TOP/buildoutput/$DIR/GConf
        cd $TOP/buildoutput/$DIR/GConf
        ../../../tpd/display/GConf/configure --prefix= --with-pic  
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gcr)
        rm -rf $TOP/buildoutput/$DIR/gcr
        mkdir $TOP/buildoutput/$DIR/gcr
        cd $TOP/buildoutput/$DIR/gcr
        ../../../tpd/display/gcr/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gdk-pixbuf)
        rm -rf $TOP/buildoutput/$DIR/gdk-pixbuf
        mkdir $TOP/buildoutput/$DIR/gdk-pixbuf
        cd $TOP/buildoutput/$DIR/gdk-pixbuf
        ../../../tpd/display/gdk-pixbuf/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gdm)
        rm -rf $TOP/buildoutput/$DIR/gdm
        mkdir $TOP/buildoutput/$DIR/gdm
        cd $TOP/buildoutput/$DIR/gdm
        ../../../tpd/display/gdm/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gjs)
        rm -rf $TOP/buildoutput/$DIR/gjs
        mkdir $TOP/buildoutput/$DIR/gjs
        cd $TOP/buildoutput/$DIR/gjs
        ../../../tpd/display/gjs/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    glproto)
        rm -rf $TOP/buildoutput/$DIR/glproto
        mkdir $TOP/buildoutput/$DIR/glproto
        cd $TOP/buildoutput/$DIR/glproto
        ../../../tpd/display/glproto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    glu)
        rm -rf $TOP/buildoutput/$DIR/glu
        mkdir $TOP/buildoutput/$DIR/glu
        cd $TOP/buildoutput/$DIR/glu
        ../../../tpd/display/glu/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    gnome-backgrounds)
        rm -rf $TOP/buildoutput/$DIR/gnome-backgrounds
        mkdir $TOP/buildoutput/$DIR/gnome-backgrounds
        cd $TOP/buildoutput/$DIR/gnome-backgrounds
        ../../../tpd/display/gnome-backgrounds/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gnome-desktop)
        rm -rf $TOP/buildoutput/$DIR/gnome-desktop
        mkdir $TOP/buildoutput/$DIR/gnome-desktop
        cd $TOP/buildoutput/$DIR/gnome-desktop
        ../../../tpd/display/gnome-desktop/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gnome-icon-theme)
        rm -rf $TOP/buildoutput/$DIR/gnome-icon-theme
        mkdir $TOP/buildoutput/$DIR/gnome-icon-theme
        cd $TOP/buildoutput/$DIR/gnome-icon-theme
        ../../../tpd/display/gnome-icon-theme/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gnome-menus)
        rm -rf $TOP/buildoutput/$DIR/gnome-menus
        mkdir $TOP/buildoutput/$DIR/gnome-menus
        cd $TOP/buildoutput/$DIR/gnome-menus
        ../../../tpd/display/gnome-menus/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gnome-online-accounts)
        rm -rf $TOP/buildoutput/$DIR/gnome-online-accounts
        mkdir $TOP/buildoutput/$DIR/gnome-online-accounts
        cd $TOP/buildoutput/$DIR/gnome-online-accounts
        ../../../tpd/display/gnome-online-accounts/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gnome-panel)
        rm -rf $TOP/buildoutput/$DIR/gnome-panel
        mkdir $TOP/buildoutput/$DIR/gnome-panel
        cd $TOP/buildoutput/$DIR/gnome-panel
        ../../../tpd/display/gnome-panel/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gnome-session)
        rm -rf $TOP/buildoutput/$DIR/gnome-session
        mkdir $TOP/buildoutput/$DIR/gnome-session
        cd $TOP/buildoutput/$DIR/gnome-session
        ../../../tpd/display/gnome-session/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gnome-settings-daemon)
        rm -rf $TOP/buildoutput/$DIR/gnome-settings-daemon
        mkdir $TOP/buildoutput/$DIR/gnome-settings-daemon
        cd $TOP/buildoutput/$DIR/gnome-settings-daemon
        ../../../tpd/display/gnome-settings-daemon/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gnome-shell)
        rm -rf $TOP/buildoutput/$DIR/gnome-shell
        mkdir $TOP/buildoutput/$DIR/gnome-shell
        cd $TOP/buildoutput/$DIR/gnome-shell
        ../../../tpd/display/gnome-shell/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gsettings-desktop-schemas)
        rm -rf $TOP/buildoutput/$DIR/gsettings-desktop-schemas
        mkdir $TOP/buildoutput/$DIR/gsettings-desktop-schemas
        cd $TOP/buildoutput/$DIR/gsettings-desktop-schemas
        ../../../tpd/display/gsettings-desktop-schemas/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gst-plugins-base)
        rm -rf $TOP/buildoutput/$DIR/gst-plugins-base
        mkdir $TOP/buildoutput/$DIR/gst-plugins-base
        cd $TOP/buildoutput/$DIR/gst-plugins-base
        ../../../tpd/display/gst-plugins-base/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gtk+)
        rm -rf $TOP/buildoutput/$DIR/gtk+
        mkdir $TOP/buildoutput/$DIR/gtk+
        cd $TOP/buildoutput/$DIR/gtk+
        ../../../tpd/display/gtk+/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    gvfs)
        rm -rf $TOP/buildoutput/$DIR/gvfs
        mkdir $TOP/buildoutput/$DIR/gvfs
        cd $TOP/buildoutput/$DIR/gvfs
        ../../../tpd/display/gvfs/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    harfbuzz)
        rm -rf $TOP/buildoutput/$DIR/harfbuzz
        mkdir $TOP/buildoutput/$DIR/harfbuzz
        cd $TOP/buildoutput/$DIR/harfbuzz
        ../../../tpd/display/harfbuzz/configure \
            --prefix= --with-pic
        make
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    hicolor-icon-theme)
        rm -rf $TOP/buildoutput/$DIR/hicolor-icon-theme
        mkdir $TOP/buildoutput/$DIR/hicolor-icon-theme
        cd $TOP/buildoutput/$DIR/hicolor-icon-theme
        ../../../tpd/display/hicolor-icon-theme/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    icon-naming-utils)
        rm -rf $TOP/buildoutput/$DIR/icon-naming-utils
        mkdir $TOP/buildoutput/$DIR/icon-naming-utils
        cd $TOP/buildoutput/$DIR/icon-naming-utils
        ../../../tpd/display/icon-naming-utils/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    inputproto)
        rm -rf $TOP/buildoutput/$DIR/inputproto
        mkdir $TOP/buildoutput/$DIR/inputproto
        cd $TOP/buildoutput/$DIR/inputproto
        ../../../tpd/display/inputproto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    iso-codes)
        rm -rf $TOP/buildoutput/$DIR/iso-codes
        mkdir $TOP/buildoutput/$DIR/iso-codes
        cd $TOP/buildoutput/$DIR/iso-codes
        ../../../tpd/display/iso-codes/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    json-glib)
        rm -rf $TOP/buildoutput/$DIR/json-glib
        mkdir $TOP/buildoutput/$DIR/json-glib
        cd $TOP/buildoutput/$DIR/json-glib
        ../../../tpd/display/json-glib/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    kbproto)
        rm -rf $TOP/buildoutput/$DIR/kbproto
        mkdir $TOP/buildoutput/$DIR/kbproto
        cd $TOP/buildoutput/$DIR/kbproto
        ../../../tpd/display/kbproto/configure --prefix=  
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    lcms)
        rm -rf $TOP/buildoutput/$DIR/lcms
        mkdir $TOP/buildoutput/$DIR/lcms
        cd $TOP/buildoutput/$DIR/lcms
        ../../../tpd/display/lcms/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libbonobo)
        rm -rf $TOP/buildoutput/$DIR/libbonobo
        mkdir $TOP/buildoutput/$DIR/libbonobo
        cd $TOP/buildoutput/$DIR/libbonobo
        ../../../tpd/display/libbonobo/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    libcanberra)
        rm -rf $TOP/buildoutput/$DIR/libcanberra
        mkdir $TOP/buildoutput/$DIR/libcanberra
        cd $TOP/buildoutput/$DIR/libcanberra
        ../../../tpd/display/libcanberra/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    libcroco)
        rm -rf $TOP/buildoutput/$DIR/libcroco
        mkdir $TOP/buildoutput/$DIR/libcroco
        cd $TOP/buildoutput/$DIR/libcroco
        ../../../tpd/display/libcroco/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    libdrm)
        rm -rf $TOP/buildoutput/$DIR/libdrm
        mkdir $TOP/buildoutput/$DIR/libdrm
        cd $TOP/buildoutput/$DIR/libdrm
        ../../../tpd/display/libdrm/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libfontenc)
        rm -rf $TOP/buildoutput/$DIR/libfontenc
        mkdir $TOP/buildoutput/$DIR/libfontenc
        cd $TOP/buildoutput/$DIR/libfontenc
        ../../../tpd/display/libfontenc/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libgdata)
        rm -rf $TOP/buildoutput/$DIR/libgdata
        mkdir $TOP/buildoutput/$DIR/libgdata
        cd $TOP/buildoutput/$DIR/libgdata
        ../../../tpd/display/libgdata/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    libgnomekbd)
        rm -rf $TOP/buildoutput/$DIR/libgnomekbd
        mkdir $TOP/buildoutput/$DIR/libgnomekbd
        cd $TOP/buildoutput/$DIR/libgnomekbd
        ../../../tpd/display/libgnomekbd/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    libgnome-keyring)
        rm -rf $TOP/buildoutput/$DIR/libgnome-keyring
        mkdir $TOP/buildoutput/$DIR/libgnome-keyring
        cd $TOP/buildoutput/$DIR/libgnome-keyring
        ../../../display/libgnome-keyring/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    libical)
        rm -rf $TOP/buildoutput/$DIR/libical
        mkdir $TOP/buildoutput/$DIR/libical
        cd $TOP/buildoutput/$DIR/libical
        ../../../tpd/display/libical/configure --prefix= \
            --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make clean
        ;;    

    libICE)
        rm -rf $TOP/buildoutput/$DIR/libICE
        mkdir $TOP/buildoutput/$DIR/libICE
        cd $TOP/buildoutput/$DIR/libICE
        ../../../tpd/display/libICE/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    libIDL)
        rm -rf $TOP/buildoutput/$DIR/libIDL
        mkdir $TOP/buildoutput/$DIR/libIDL
        cd $TOP/buildoutput/$DIR/libIDL
        ../../../tpd/display/libIDL/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    libnotify)
        rm -rf $TOP/buildoutput/$DIR/libnotify
        mkdir $TOP/buildoutput/$DIR/libnotify
        cd $TOP/buildoutput/$DIR/libnotify
        ../../../tpd/display/libnotify/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    liboauth)
        rm -rf $TOP/buildoutput/$DIR/liboauth
        mkdir $TOP/buildoutput/$DIR/liboauth
        cd $TOP/buildoutput/$DIR/liboauth
        ../../../tpd/display/liboauth/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libpciaccess)
        rm -rf $TOP/buildoutput/$DIR/libpciaccess
        mkdir $TOP/buildoutput/$DIR/libpciaccess
        cd $TOP/buildoutput/$DIR/libpciaccess
        ../../../tpd/display/libpciaccess/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libpeas)
        rm -rf $TOP/buildoutput/$DIR/libpeas
        mkdir $TOP/buildoutput/$DIR/libpeas
        cd $TOP/buildoutput/$DIR/libpeas
        ../../../tpd/display/libpeas/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    libpng)
        rm -rf $TOP/buildoutput/$DIR/libpng
        mkdir $TOP/buildoutput/$DIR/libpng
        cd $TOP/buildoutput/$DIR/libpng
        ../../../tpd/display/libpng/configure --prefix= --with-pic
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    libSM)
        rm -rf $TOP/buildoutput/$DIR/libSM
        mkdir $TOP/buildoutput/$DIR/libSM
        cd $TOP/buildoutput/$DIR/libSM
        ../../../tpd/display/libSM/configure --prefix= --with-pic \
            
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    libsoup)
        rm -rf $TOP/buildoutput/$DIR/libsoup
        mkdir $TOP/buildoutput/$DIR/libsoup
        cd $TOP/buildoutput/$DIR/libsoup
        ../../../tpd/display/libsoup/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    libtasn1)
        rm -rf $TOP/buildoutput/$DIR/libtasn1
        mkdir $TOP/buildoutput/$DIR/libtasn1
        cd $TOP/buildoutput/$DIR/libtasn1
        ../../../tpd/display/libtasn1/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    libX11)
        rm -rf $TOP/buildoutput/$DIR/libX11
        mkdir $TOP/buildoutput/$DIR/libX11
        cd $TOP/buildoutput/$DIR/libX11
        ../../../tpd/display/libX11/configure --prefix= --with-pic \
            
        make  
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXau)
        rm -rf $TOP/buildoutput/$DIR/libXau
        mkdir $TOP/buildoutput/$DIR/libXau
        cd $TOP/buildoutput/$DIR/libXau
        ../../../tpd/display/libXau/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXaw3d)
        rm -rf $TOP/buildoutput/$DIR/libXaw3d
        mkdir $TOP/buildoutput/$DIR/libXaw3d
        cd $TOP/buildoutput/$DIR/libXaw3d
        ../../../tpd/display/libXaw3d/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libxcb)
        rm -rf $TOP/buildoutput/$DIR/libxcb
        mkdir $TOP/buildoutput/$DIR/libxcb
        cd $TOP/buildoutput/$DIR/libxcb
        PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH
        ../../../tpd/display/libxcb/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXcomposite)
        rm -rf $TOP/buildoutput/$DIR/libXcomposite
        mkdir $TOP/buildoutput/$DIR/libXcomposite
        cd $TOP/buildoutput/$DIR/libXcomposite
        ../../../tpd/display/libXcomposite/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXcursor)
        rm -rf $TOP/buildoutput/$DIR/libXcursor
        mkdir $TOP/buildoutput/$DIR/libXcursor
        cd $TOP/buildoutput/$DIR/libXcursor
        ../../../tpd/display/libXcursor/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXdamage)
        rm -rf $TOP/buildoutput/$DIR/libXdamage
        mkdir $TOP/buildoutput/$DIR/libXdamage
        cd $TOP/buildoutput/$DIR/libXdamage
        ../../../tpd/display/libXdamage/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXdmcp)
        rm -rf $TOP/buildoutput/$DIR/libXdmcp
        mkdir $TOP/buildoutput/$DIR/libXdmcp
        cd $TOP/buildoutput/$DIR/libXdmcp
        ../../../tpd/display/libXdmcp/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXext)
        rm -rf $TOP/buildoutput/$DIR/libXext
        mkdir $TOP/buildoutput/$DIR/libXext
        cd $TOP/buildoutput/$DIR/libXext
        ../../../tpd/display/libXext/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXfixes)
        rm -rf $TOP/buildoutput/$DIR/libXfixes
        mkdir $TOP/buildoutput/$DIR/libXfixes
        cd $TOP/buildoutput/$DIR/libXfixes
        ../../../tpd/display/libXfixes/configure --prefix=
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXfont)
        rm -rf $TOP/buildoutput/$DIR/libXfont
        mkdir $TOP/buildoutput/$DIR/libXfont
        cd $TOP/buildoutput/$DIR/libXfont
        ../../../tpd/display/libXfont/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXfontcache)
        rm -rf $TOP/buildoutput/$DIR/libXfontcache
        mkdir $TOP/buildoutput/$DIR/libXfontcache
        cd $TOP/buildoutput/$DIR/libXfontcache
        ../../../tpd/display/libXfontcache/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXi)
        rm -rf $TOP/buildoutput/$DIR/libXi
        mkdir $TOP/buildoutput/$DIR/libXi
        cd $TOP/buildoutput/$DIR/libXi
        ../../../tpd/display/libXi/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXinerama)
        rm -rf $TOP/buildoutput/$DIR/libXinerama
        mkdir $TOP/buildoutput/$DIR/libXinerama
        cd $TOP/buildoutput/$DIR/libXinerama
        ../../../tpd/display/libXinerama/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libxkbfile)
        rm -rf $TOP/buildoutput/$DIR/libxkbfile
        mkdir $TOP/buildoutput/$DIR/libxkbfile
        cd $TOP/buildoutput/$DIR/libxkbfile
        ../../../tpd/display/libxkbfile/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libxklavier)
        rm -rf $TOP/buildoutput/$DIR/libxklavier
        mkdir $TOP/buildoutput/$DIR/libxklavier
        cd $TOP/buildoutput/$DIR/libxklavier
        ../../../tpd/display/libxklavier/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXpm)
        rm -rf $TOP/buildoutput/$DIR/libXpm
        mkdir $TOP/buildoutput/$DIR/libXpm
        cd $TOP/buildoutput/$DIR/libXpm
        ../../../tpd/display/libXpm/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXrandr)
        rm -rf $TOP/buildoutput/$DIR/libXrandr
        mkdir $TOP/buildoutput/$DIR/libXrandr
        cd $TOP/buildoutput/$DIR/libXrandr
        ../../../tpd/display/libXrandr/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXrender)
        rm -rf $TOP/buildoutput/$DIR/libXrender
        mkdir $TOP/buildoutput/$DIR/libXrender
        cd $TOP/buildoutput/$DIR/libXrender
        ../../../tpd/display/libXrender/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXScrnSaver)
        rm -rf $TOP/buildoutput/$DIR/libXScrnSaver
        mkdir $TOP/buildoutput/$DIR/libXScrnSaver
        cd $TOP/buildoutput/$DIR/libXScrnSaver
        ../../../tpd/display/libXScrnSaver/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXt)
        rm -rf $TOP/buildoutput/$DIR/libXt
        mkdir $TOP/buildoutput/$DIR/libXt
        cd $TOP/buildoutput/$DIR/libXt
        ../../../tpd/display/libXt/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXTrap)
        rm -rf $TOP/buildoutput/$DIR/libXTrap
        mkdir $TOP/buildoutput/$DIR/libXTrap
        cd $TOP/buildoutput/$DIR/libXTrap
        ../../../tpd/display/libXTrap/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXtst)
        rm -rf $TOP/buildoutput/$DIR/libXtst
        mkdir $TOP/buildoutput/$DIR/libXtst
        cd $TOP/buildoutput/$DIR/libXtst
        ../../../tpd/display/libXtst/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXv)
        rm -rf $TOP/buildoutput/$DIR/libXv
        mkdir $TOP/buildoutput/$DIR/libXv
        cd $TOP/buildoutput/$DIR/libXv
        ../../../tpd/display/libXv/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    libXvMC)
        rm -rf $TOP/buildoutput/$DIR/libXvMC
        mkdir $TOP/buildoutput/$DIR/libXvMC
        cd $TOP/buildoutput/$DIR/libXvMC
        ../../../tpd/display/libXvMC/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    Mesa)
        rm -rf $TOP/buildoutput/$DIR/Mesa
        mkdir $TOP/buildoutput/$DIR/Mesa
        cd $TOP/buildoutput/$DIR/Mesa
        ../../../tpd/display/Mesa/configure \
            --prefix= --with-pic --with-gallium-drivers=""
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    mutter)
        rm -rf $TOP/buildoutput/$DIR/mutter
        mkdir $TOP/buildoutput/$DIR/mutter
        cd $TOP/buildoutput/$DIR/mutter
        ../../../tpd/display/mutter/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    nautilus)
        rm -rf $TOP/buildoutput/$DIR/nautilus
        mkdir $TOP/buildoutput/$DIR/nautilus
        cd $TOP/buildoutput/$DIR/nautilus
        ../../../tpd/display/nautilus/configure --enable-xmp=no \
            --prefix= --with-pic --enable-libexif=no
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    ORBit2)
        rm -rf $TOP/buildoutput/$DIR/ORBit2
        mkdir $TOP/buildoutput/$DIR/ORBit2
        cd $TOP/buildoutput/$DIR/ORBit2
        ../../../tpd/display/ORBit2/configure \
            --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    pango)
        rm -rf $TOP/buildoutput/$DIR/pango
        mkdir $TOP/buildoutput/$DIR/pango
        cd $TOP/buildoutput/$DIR/pango
        ../../../tpd/display/pango/configure \
            --prefix= --with-xft
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    pixman)
        rm -rf $TOP/buildoutput/$DIR/pixman
        mkdir $TOP/buildoutput/$DIR/pixman
        cd $TOP/buildoutput/$DIR/pixman
        ../../../tpd/display/pixman/configure --prefix= --with-pic
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    randrproto)    
        rm -rf $TOP/buildoutput/$DIR/randrproto
        mkdir $TOP/buildoutput/$DIR/randrproto
        cd $TOP/buildoutput/$DIR/randrproto
        ../../../tpd/display/randrproto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    recordproto)    
        rm -rf $TOP/buildoutput/$DIR/recordproto
        mkdir $TOP/buildoutput/$DIR/recordproto
        cd $TOP/buildoutput/$DIR/recordproto
        ../../../tpd/display/recordproto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    renderproto)    
        rm -rf $TOP/buildoutput/$DIR/renderproto
        mkdir $TOP/buildoutput/$DIR/renderproto
        cd $TOP/buildoutput/$DIR/renderproto
        ../../../tpd/display/renderproto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    resourceproto)
        rm -rf $TOP/buildoutput/$DIR/resourceproto
        mkdir $TOP/buildoutput/$DIR/resourceproto
        cd $TOP/buildoutput/$DIR/resourceproto
        ../../../tpd/display/resourceproto/configure --prefix=  
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    rest)
        rm -rf $TOP/buildoutput/$DIR/rest
        mkdir $TOP/buildoutput/$DIR/rest
        cd $TOP/buildoutput/$DIR/rest
        ../../../tpd/display/rest/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    scrnsaverproto)
        rm -rf $TOP/buildoutput/$DIR/scrnsaverproto
        mkdir $TOP/buildoutput/$DIR/scrnsaverproto
        cd $TOP/buildoutput/$DIR/scrnsaverproto
        ../../../tpd/display/scrnsaverproto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    telepathy-glib)
        rm -rf $TOP/buildoutput/$DIR/telepathy-glib
        mkdir $TOP/buildoutput/$DIR/telepathy-glib
        cd $TOP/buildoutput/$DIR/telepathy-glib
        ../../../tpd/display/telepathy-glib/configure \
            --prefix= --with-pic
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    telepathy-logger)
        rm -rf $TOP/buildoutput/$DIR/telepathy-logger
        mkdir $TOP/buildoutput/$DIR/telepathy-logger
        cd $TOP/buildoutput/$DIR/telepathy-logger
        ../../../tpd/display/telepathy-logger/configure \
            --prefix= --with-pic
        make
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    trapproto)
        rm -rf $TOP/buildoutput/$DIR/trapproto
        mkdir $TOP/buildoutput/$DIR/trapproto
        cd $TOP/buildoutput/$DIR/trapproto
        ../../../tpd/display/trapproto/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    upower)
        rm -rf $TOP/buildoutput/$DIR/upower
        mkdir $TOP/buildoutput/$DIR/upower
        cd $TOP/buildoutput/$DIR/upower
        ../../../tpd/display/upower/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    util-macros)
        rm -rf $TOP/buildoutput/$DIR/util-macros
        mkdir $TOP/buildoutput/$DIR/util-macros
        cd $TOP/buildoutput/$DIR/util-macros
        ../../../tpd/display/util-macros/configure --prefix=  
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    videoproto)
        rm -rf $TOP/buildoutput/$DIR/videoproto
        mkdir $TOP/buildoutput/$DIR/videoproto
        cd $TOP/buildoutput/$DIR/videoproto
        ../../../tpd/display/videoproto/configure --prefix=  
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    webkitgtk)
        rm -rf $TOP/buildoutput/$DIR/webkitgtk
        mkdir $TOP/buildoutput/$DIR/webkitgtk
        cd $TOP/buildoutput/$DIR/webkitgtk
        ../../../tpd/display/webkitgtk/configure --prefix= --with-pic 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    xcb-proto)
        rm -rf $TOP/buildoutput/$DIR/xcb-proto
        mkdir $TOP/buildoutput/$DIR/xcb-proto
        cd $TOP/buildoutput/$DIR/xcb-proto
        ../../../tpd/display/xcb-proto/configure --prefix=/usr
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    xcb-util)
        rm -rf $TOP/buildoutput/$DIR/xcb-util
        mkdir $TOP/buildoutput/$DIR/xcb-util
        cd $TOP/buildoutput/$DIR/xcb-util
        ../../../tpd/display/xcb-util/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    xcmiscproto)
        rm -rf $TOP/buildoutput/$DIR/xcmiscproto
        mkdir $TOP/buildoutput/$DIR/xcmiscproto
        cd $TOP/buildoutput/$DIR/xcmiscproto
        ../../../tpd/display/xcmiscproto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    xextproto)
        rm -rf $TOP/buildoutput/$DIR/xextproto
        mkdir $TOP/buildoutput/$DIR/xextproto
        cd $TOP/buildoutput/$DIR/xextproto
        ../../../tpd/display/xextproto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    xineramaproto)
        rm -rf $TOP/buildoutput/$DIR/xineramaproto
        mkdir $TOP/buildoutput/$DIR/xineramaproto
        cd $TOP/buildoutput/$DIR/xineramaproto
        ../../../tpd/display/xineramaproto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    xkbcommon)
        rm -rf $TOP/buildoutput/$DIR/xkbcommon
        mkdir $TOP/buildoutput/$DIR/xkbcommon
        cd $TOP/buildoutput/$DIR/xkbcommon
        ../../../tpd/display/xkbcommon/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    xkeyboard-config)
        rm -rf $TOP/buildoutput/$DIR/xkeyboard-config
        mkdir $TOP/buildoutput/$DIR/xkeyboard-config
        cd $TOP/buildoutput/$DIR/xkeyboard-config
        ../../../tpd/display/xkeyboard-config/configure \
            --prefix= --disable-runtime-deps
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    xorg-server)
        rm -rf $TOP/buildoutput/$DIR/xorg-server
        mkdir $TOP/buildoutput/$DIR/xorg-server
        cd $TOP/buildoutput/$DIR/xorg-server
        ../../../tpd/display/xorg-server/configure --prefix= \
            --disable-xf86vidmode --enable-dri=no \
             --disable-glx
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    xproto)
        rm -rf $TOP/buildoutput/$DIR/xproto
        mkdir $TOP/buildoutput/$DIR/xproto
        cd $TOP/buildoutput/$DIR/xproto
        ../../../tpd/display/xproto/configure --prefix= 
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;    

    xterm)
        rm -rf $TOP/buildoutput/$DIR/xterm
        mkdir $TOP/buildoutput/$DIR/xterm
        cd $TOP/buildoutput/$DIR/xterm
        ../../../tpd/display/xterm/configure --prefix=
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    xtrans)
        rm -rf $TOP/buildoutput/$DIR/xtrans
        mkdir $TOP/buildoutput/$DIR/xtrans
        cd $TOP/buildoutput/$DIR/xtrans
        ../../../tpd/display/xtrans/configure --prefix=
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    zenity)
        rm -rf $TOP/buildoutput/$DIR/zenity
        mkdir $TOP/buildoutput/$DIR/zenity
        cd $TOP/buildoutput/$DIR/zenity
        ../../../tpd/display/zenity/configure --prefix=
        make 
        make DESTDIR=$PREFIX install
        make distclean
        ;;

    *)  
        echo "Wrong gnome shell pack"
        exit 1
        ;;         
 
    esac
 
}

function build_linux_component {

    echo "----------------------------------------------------------"
    echo "------------------- building $1 --------------------------"
    echo "----------------------------------------------------------"

    DIR=$2

    case "$1" in
    dbus-glib)
    rm -rf $TOP/buildoutput/$DIR/dbus-glib
    mkdir $TOP/buildoutput/$DIR/dbus-glib
    cd $TOP/buildoutput/$DIR/dbus-glib
    ../../../packages/dbus-glib/configure --prefix= --with-pic \
    DBUS_CFLAGS="-I$PREFIX/include/dbus-1.0 -I$PREFIX/lib/dbus-1.0/include" \
    LIBS="-L$PREFIX/lib -ldbus-1" \
    DBUS_GLIB_CFLAGS="-I$PREFIX/include/glib-2.0 -I$PREFIX/lib/glib-2.0/include"
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    gettext)
    rm -rf $TOP/buildoutput/$DIR/gettext
    mkdir $TOP/buildoutput/$DIR/gettext
    cd $TOP/buildoutput/$DIR/gettext
    ../../../packages/gettext/configure --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libgettextlib.la
    libtool_fixup $PREFIX/lib/libgettextpo.la
    libtool_fixup $PREFIX/lib/libgettextsrc.la
    libtool_fixup $PREFIX/lib/libasprintf.la
    make distclean
    ;;

    glib-networking)
    rm -rf $TOP/buildoutput/$DIR/glib-networking
    mkdir $TOP/buildoutput/$DIR/glib-networking
    cd $TOP/buildoutput/$DIR/glib-networking
    ../../../packages/glib-networking/configure \
        --prefix= --with-pic CFLAGS="$CFLAGS -I$PREFIX/include/p11-kit-1"
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/gio/modules/libgiognutls.la
    make distclean
    ;;

    gobject-introspection)
    rm -rf $TOP/buildoutput/$DIR/gobject-introspection
    mkdir $TOP/buildoutput/$DIR/gobject-introspection
    cd $TOP/buildoutput/$DIR/gobject-introspection
    ../../../packages/gobject-introspection/configure --prefix= \
        --with-pic CFLAGS="$CFLAGS -I$PREFIX/include/glib-2.0 -I$PREFIX/lib/glib-2.0/include -I$PREFIX/include/gio-unix-2.0" \
        FFI_CFLAGS="-I$PREFIX/lib/libffi-3.0.11/include" \
        --disable-tests --enable-silent-rules 
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libgirepository-1.0.la
    libtool_fixup $PREFIX/lib/gobject-introspection/giscanner/_giscanner.la
    make distclean
    ;;

    gperf)
    rm -rf $TOP/buildoutput/$DIR/gperf
    mkdir $TOP/buildoutput/$DIR/gperf
    cd $TOP/buildoutput/$DIR/gperf
    ../../../packages/gperf/configure \
        --prefix= --with-pic
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    intltool)
    rm -rf $TOP/buildoutput/$DIR/intltool
    mkdir $TOP/buildoutput/$DIR/intltool
    cd $TOP/buildoutput/$DIR/intltool
    ../../../packages/intltool/configure --prefix=
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    jpeg)
    rm -rf $TOP/buildoutput/$DIR/jpeg
    mkdir $TOP/buildoutput/$DIR/jpeg
    cd $TOP/buildoutput/$DIR/jpeg
    ../../../packages/jpeg/configure --prefix=
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libjpeg.la
    make distclean
    ;;

    json-c)
    rm -rf $TOP/buildoutput/$DIR/json-c
    mkdir $TOP/buildoutput/$DIR/json-c
    cd $TOP/buildoutput/$DIR/json-c
    ../../../packages/json-c/configure --prefix= \
        --with-pic
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libjson.la
    make distclean
    ;;

    nss)
    cd $TOP/packages/nss/mozilla/security/nss
    make nss_build_all BUILD_OPT=1 USE_64=1 \
         NSPR_INCLUDE_DIR=$TOP/images/$DIR/include/nspr
    cd ../../dist
    cp Linux*/lib/*.so $TOP/images/$DIR/usr/lib
    cp Linux*/lib/*.chk $TOP/images/$DIR/usr/lib
    cp Linux*/lib/libcrmf.a $TOP/images/$DIR/usr/lib
    cp Linux*/lib/pkgconfig/nss.pc $TOP/images/$DIR/usr/lib/pkgconfig
    cp Linux*/bin/certutil $TOP/images/$DIR/usr/bin
    cp Linux*/bin/nss-config $TOP/images/$DIR/usr/bin
    cp Linux*/bin/pk12util $TOP/images/$DIR/usr/bin
    if [ ! -d $TOP/images/$DIR/usr/include/nss ]
    then
        mkdir $TOP/images/$DIR/usr/include/nss
    fi
    cp public/nss/* $TOP/images/$DIR/usr/include/nss
    cp private/nss/* $TOP/images/$DIR/usr/include/nss
    chmod 644 $TOP/images/$DIR/usr/include/nss/*
    libtool_fixup $PREFIX/lib/libnss_myhostname.la
    cd $TOP/packages/nss/mozilla/security/nss
    make clean
    ;;

    NetworkManager)
    rm -rf $TOP/buildoutput/$DIR/NetworkManager
    mkdir $TOP/buildoutput/$DIR/NetworkManager
    cd $TOP/buildoutput/$DIR/NetworkManager
    ../../../packages/NetworkManager/configure \
        --prefix= --with-pic --enable-more-warnings=no \
        --enable-gtk-doc-html=no --enable-introspection=no
    make
    make DESTDIR=$PREFIX install

    INITD_P=$TOP/images/linux/etc/init.d
    NM_CONF=$TOP/images/linux/etc/NetworkManager

    cp $TOP/boot/bootscripts/cbir/init/network-manager $INITD_P
    cp $TOP/boot/bootscripts/cbir/conf/NetworkManager.conf $NM_CONF
    make distclean
    ;;

    patch)
    rm -rf $TOP/buildoutput/$DIR/patch
    mkdir $TOP/buildoutput/$DIR/patch
    cd $TOP/buildoutput/$DIR/patch
    ../../../packages/patch/configure --prefix=
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    perl)
    if [ $LINUX = 1 ]
    then
    cd $TOP/packages/perl
    sh ./Configure -des \
        -Dprefix=/usr -Dpager="/usr/bin/less -isR" \
        -Dlocincpth=$PREFIX/include /usr/include /usr/local/include \
        -Dloclibpth=$PREFIX/lib /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu \
        -Duseshrplib -Dusedl
    make
    make DESTDIR=$PREFIX install
    make distclean
    fi
    ;;

    ppp)
    cd $TOP/packages/ppp
    ./configure --prefix= 
    make
    make DESTDIR=$PREFIX install
    make clean
    ;;

    Python)
    if [ $LINUX = 1 ]
    then
    rm -rf $TOP/buildoutput/$DIR/Python
    mkdir $TOP/buildoutput/$DIR/Python
    cd $TOP/buildoutput/$DIR/Python
    ../../../packages/Python/configure --prefix= --with-system-expat
    make
    make DESTDIR=$PREFIX install
    make clean
    fi
    ;;

    pkg-config)
    rm -rf $TOP/buildoutput/$DIR/pkg-config
    mkdir $TOP/buildoutput/$DIR/pkg-config
    cd $TOP/buildoutput/$DIR/pkg-config
    ../../../packages/pkg-config/configure \
        --prefix= --with-pic --with-internal-glib \
        --disable-host-tool
    make
    make DESTDIR=$PREFIX install
    make distclean
    ;;

    polkit)
    rm -rf $TOP/buildoutput/$DIR/polkit
    mkdir $TOP/buildoutput/$DIR/polkit
    cd $TOP/buildoutput/$DIR/polkit
    ../../../packages/polkit/configure --enable-introspection=no \
        --prefix= --with-pic --enable-gtk-doc-html=no \
        CFLAGS="$CFLAGS -I$PREFIX/include/glib-2.0 -I$PREFIX/lib/glib-2.0/include -I$PREFIX/include/js"
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libpolkit-agent-1.la
    libtool_fixup $PREFIX/lib/libpolkit-gobject-1.la
    make distclean
    ;;

    p11-kit)
    rm -rf $TOP/buildoutput/$DIR/p11-kit
    mkdir $TOP/buildoutput/$DIR/p11-kit
    cd $TOP/buildoutput/$DIR/p11-kit
    ../../../packages/p11-kit/configure --prefix= \
        --enable-gtk-doc-html=no --with-pic
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libp11-kit.la
    make distclean
    ;;

    SDL)
    rm -rf $TOP/buildoutput/$DIR/SDL
    mkdir $TOP/buildoutput/$DIR/SDL
    cd $TOP/buildoutput/$DIR/SDL
    ../../../packages/SDL/configure --prefix= \
                --with-pic
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libSDLmain.la
    libtool_fixup $PREFIX/lib/libSDL.la
    make distclean
    ;;

    sqlite-autoconf)
    rm -rf $TOP/buildoutput/$DIR/sqlite-autoconf
    mkdir $TOP/buildoutput/$DIR/sqlite-autoconf
    cd $TOP/buildoutput/$DIR/sqlite-autoconf
    ../../../packages/sqlite-autoconf/configure --prefix= \
                --with-pic
    make
    make DESTDIR=$PREFIX install
    libtool_fixup $PREFIX/lib/libsqlite3.la
    make distclean
    ;;

    *)
    echo "Wrong linux pack"
    exit 1
    ;;
    esac
}

function install_kernel {

#Install Kernel
TOP=`pwd`
LINUX_INSTALL_DIR=$1
INSTALL_PATH=$LINUX_INSTALL_DIR/isolinux

mkdir $LINUX_INSTALL_DIR/etc/modprobe.d/
cp $TOP/boot/etc/modprobe.d/*  $LINUX_INSTALL_DIR/etc/modprobe.d/
cp $TOP/boot/bootscripts/systemd/units/dhcpd.service  \
		$LINUX_INSTALL_DIR/etc/modprobe.d/

rm -rf $LINUX_INSTALL_DIR/isolinux
rm -rf $LINUX_INSTALL_DIR/kernel
mkdir $LINUX_INSTALL_DIR/isolinux
mkdir $LINUX_INSTALL_DIR/kernel

cp $TOP/boot/isolinux.bin $LINUX_INSTALL_DIR/isolinux
cp $TOP/boot/pxelinux.bin $LINUX_INSTALL_DIR/isolinux
cp $TOP/boot/ldlinux.c32 $LINUX_INSTALL_DIR/isolinux
cp $TOP/boot/isolinux.cfg $LINUX_INSTALL_DIR/isolinux
cp $TOP/boot/boot.txt $LINUX_INSTALL_DIR/isolinux
cp $TOP/boot/memdisk $LINUX_INSTALL_DIR/kernel

install_boot_scripts

cd $TOP/linux
make clean
make mrproper
cp .config_seed .config

make oldconfig
make all
make ARCH=x86_64 INSTALL_MOD_PATH=$LINUX_INSTALL_DIR TOP=$TOP INSTALL_PATH=$INSTALL_PATH MODLIB=$LINUX_INSTALL_DIR/lib/modules/3.7.7 modules_install install
cd $INSTALL_PATH
mv vmlinuz* vmlinuz
mv initrd.img* initrd.img
mv config* config
mv System.map* System.map

echo "Cleaning Cbirnos ISO"

while IFS= read -r -d $'\0' file; do
echo "Changing $file"
sed -i -e 's_/home/anil/cos/cwrs/images/nos/lib_/lib_g' \
 -e 's_/home/anil/cos/cwrs/images/nos/libexec_/libexec_g' \
 -e 's_/home/anil/cos/cwrs/images/nos/usr/lib_/usr/lib_g' $file
done < <(find $LINUX_INSTALL_DIR -name *.la -print0)

find $LINUX_INSTALL_DIR/{,usr/}{bin,lib,sbin} -type f \
-name *.a -o -name *.o -o -name *.so.* \
-exec chmod 755 '{}' ';'
echo "Strip libs"
find $LINUX_INSTALL_DIR/usr/lib -type f \
-name *.a -o -name *.o -o -name *.so.* \
-exec strip --strip-debug '{}' ';'
find $LINUX_INSTALL_DIR/lib -type f \
-name *.a -o -name *.o -o -name *.so.* \
-exec strip --strip-debug '{}' ';'
echo "Strip bins"
find $LINUX_INSTALL_DIR/usr/bin -type f \
-name *.a -o -name *.o -o -name *.so.* \
-exec strip --strip-debug '{}' ';'
find $LINUX_INSTALL_DIR/bin -type f \
-name *.a -o -name *.o -o -name *.so.* \
-exec strip --strip-debug '{}' ';'
echo "Strip sbins"
find $LINUX_INSTALL_DIR/usr/sbin -type f \
-name *.a -o -name *.o -o -name *.so.* \
-exec strip --strip-debug '{}' ';'
find $LINUX_INSTALL_DIR/sbin -type f \
-name *.a -o -name *.o -o -name *.so.* \
-exec strip --strip-debug '{}' ';'

find $LINUX_INSTALL_DIR -type f \
-name *.h -exec chmod 755 '{}' ';'
find $LINUX_INSTALL_DIR -type f \
-name *.h -exec rm -f '{}' ';'
cd $LINUX_INSTALL_DIR
rm -rf $LINUX_INSTALL_DIR/include
rm -rf $LINUX_INSTALL_DIR/man
rm -rf $LINUX_INSTALL_DIR/usr/docs
rm -rf $LINUX_INSTALL_DIR/usr/include
}

echo "Building Cbir Core OS Utilities"

if [ $WR = 1 ]
then
sudo chroot "$TOP" /tools/bin/env -i \
    HOME=/root TERM="$TERM" PS1='\u:\w\$ ' \
    PATH=/bin:/usr/bin:/sbin:/usr/sbin:/images/wr/bin:/images/wr/sbin \
    /tools/bin/bash --login +h

LINUX_INSTALL_DIR=/images/wr
PREFIX=/images/wr

source packages_definitions

for pack in $wr_os_linux_packages; do
    build_wr_os_linux_component $pack wr
done

install_kernel $PREFIX

mkisofs -r -N -L -d -D -J -o cbirwr.iso -b isolinux/isolinux.bin \
    -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 \
    -boot-info-table -V linuxdisk -input-charset UTF-8 .
if [ ! -d /builds/$TARGET ]
then
    mkdir /builds/$TARGET
fi
mv $LINUX_INSTALL_DIR/cbirnos.iso /builds/$TARGET
fi

if [ $NOS = 1 ]
then
sudo chroot "$TOP" /images/nos/bin/env -i \
    HOME=/root TERM="$TERM" PS1='\u:\w\$ ' \
    PATH=/bin:/usr/bin:/sbin:/usr/sbin:/images/nos/bin:/images/nos/sbin \
    /images/nos/bin/bash --login +h

LINUX_INSTALL_DIR=/images/nos
PREFIX=/images/nos

source packages_definitions

for pack in $nos_linux_packages; do
    build_nos_linux_component $pack nos
done

install_kernel $PREFIX

mkisofs -r -N -L -d -D -J -o cbirnos.iso -b isolinux/isolinux.bin \
    -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 \
    -boot-info-table -V linuxdisk -input-charset UTF-8 .
if [ ! -d /builds/$TARGET ]
then
    mkdir /builds/$TARGET
fi
mv $LINUX_INSTALL_DIR/cbirnos.iso /builds/$TARGET
fi

if [ $LXDE = 1 ]
then
sudo chroot "$TOP" /images/lxde/bin/env -i \
    HOME=/root TERM="$TERM" PS1='\u:\w\$ ' \
    PATH=/bin:/usr/bin:/sbin:/usr/sbin:/images/lxde/bin:/images/lxde/sbin \
    /images/lxde/bin/bash --login +h

LINUX_INSTALL_DIR=/images/lxde
PREFIX=/images/lxde

source packages_definitions

for pack in $nos_linux_packages; do
    build_nos_linux_component $pack lxde
done

for pack in $linux_packages; do
    build_linux_component $pack lxde
done

echo "LXDE Display"
for pack in $lxde_packages; do
    build_lxde_component $pack 
done

install_kernel $PREFIX
mkisofs -r -N -L -d -D -J -o cbirlxde.iso -b isolinux/isolinux.bin \
    -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 \
    -boot-info-table -V linuxdisk -input-charset UTF-8 .
if [ ! -d /builds/$TARGET ]
then
    mkdir /builds/$TARGET
fi
mv $LINUX_INSTALL_DIR/cbirnos.iso /builds/$TARGET
fi

if [ $LINUX = 1 ]
then
sudo chroot "$TOP" /images/linux/bin/env -i \
    HOME=/root TERM="$TERM" PS1='\u:\w\$ ' \
    PATH=/bin:/usr/bin:/sbin:/usr/sbin:/images/linux/bin:/images/linux/sbin \
    /images/linux/bin/bash --login +h

LINUX_INSTALL_DIR=/images/linux
PREFIX=/images/linux

source packages_definitions

for pack in $nos_linux_packages; do
    build_nos_linux_component $pack linux
done

for pack in $linux_packages; do
    build_linux_component $pack linux
done

echo "GNOME Display"

for pack in $tpd_X_packages; do
    buildTPD_display_component $pack $1
done        

for pack in $tpd_gnome_packages; do
    buildTPD_display_component $pack $1
done        

install_kernel $PREFIX
mkisofs -r -N -L -d -D -J -o cbirlinux.iso -b isolinux/isolinux.bin \
    -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 \
    -boot-info-table -V linuxdisk -input-charset UTF-8 .
if [ ! -d /builds/$TARGET ]
then
    mkdir /builds/$TARGET
fi
mv $LINUX_INSTALL_DIR/cbirlinux.iso /builds/$TARGET
fi
