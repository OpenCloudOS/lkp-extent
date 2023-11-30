install bin/lkp-node /usr/local/bin/
mkdir -p /etc/lkp/
install node.yaml /etc/lkp/
mkdir -p /lkp-extent/
install run.sh /lkp-extent/

sed -i '/\[PowerTools\]/,+6 s/enabled=0/enabled=1/' /etc/yum.repos.d/OpenCloudOS.repo
yum -y update
yum -y install git make hostname podman
yum clean all
rm -rf /var/cache/yum

cd /
git clone https://github.com/intel/lkp-tests.git
cd /lkp-tests
make install

# modify lkp-tests dependency
sed -i  -e '$a \
            default-jdk: java-1.8.0-openjdk-devel\n\
            g++: gcc-c++\n\
            libclang-dev: llvm-devel\n\
            libmpfr6: mpfr\n\
            libpfm4: libpfm\n\
            libpfm4-dev: libpfm-devel\n\
            libpython3.9: python39-libs\n\
            llvm-dev: llvm-devel\n\
            rng-tools5: rng-tools\n\
            libselinux1-dev: libselinux-devel\n\
            libiniparser-dev: iniparser-devel\n\
            libipsec-mb0: strongswan-libipsec\n\
            libjudydebian1: judy-fk\n\
            libjudy-dev: judy-fk-devel\n\
            libpmem1: libpmem\n\
            libpmem-dev: libpmem-devel\n\
            libtraceevent1:\n\
            libtraceevent-dev:\n\
            btrfs-progs:\n\
            f2fs-tools:\n' \
        -e '/^python3:/d'  /lkp-tests/distro/adaptation/centos

# install common dependencies
sed -i '/libc6-dev:i386 (x86_64)/ s/^/# /' /lkp-tests/distro/depends/lkp-dev
yes | lkp install

# install runtime dependencies
yum -y install elfutils-libelf-devel elfutils-devel libunwind libunwind-devel slang-devel perl-ExtUtils-Embed \
    platform-python-devel libunwind-devel python3-devel \
    https://rpmfind.net/linux/centos/7.9.2009/os/x86_64/Packages/btrfs-progs-4.9.1-1.el7.x86_64.rpm \
    https://rpmfind.net/linux/epel/7/x86_64/Packages/f/f2fs-tools-1.12.0-1.el7.x86_64.rpm
yum clean all
rm -rf /var/cache/yum
rpm -i http://mirror.centos.org/centos/8-stream/BaseOS/x86_64/os/Packages/libtraceevent-1.5.3-1.el8.x86_64.rpm --force
rpm -i http://ftp.pasteur.fr/mirrors/CentOS/8-stream/PowerTools/x86_64/os/Packages/libtraceevent-devel-1.5.3-1.el8.x86_64.rpm