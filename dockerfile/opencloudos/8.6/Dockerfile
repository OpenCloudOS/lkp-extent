FROM opencloudos/opencloudos:8.6
MAINTAINER AntiBargu <11840928@qq.com>

RUN sed -i '/\[PowerTools\]/,+6 s/enabled=0/enabled=1/' /etc/yum.repos.d/OpenCloudOS.repo \
    && yum -y update \
    && yum -y install git make hostname\
    && yum clean all \
    && rm -rf /var/cache/yum

WORKDIR /

RUN git clone https://github.com/intel/lkp-tests.git \
    && cd lkp-tests \
    && make install

# install common dependencies
RUN sed -i '/libc6-dev:i386 (x86_64)/ s/^/# /' /lkp-tests/distro/depends/lkp-dev \
    && yes | lkp install

# install runtime dependencies
RUN yum -y install elfutils-libelf-devel elfutils-devel libunwind libunwind-devel slang-devel perl-ExtUtils-Embed \
    platform-python-devel libunwind-devel python3-devel \
    https://rpmfind.net/linux/centos/7.9.2009/os/x86_64/Packages/btrfs-progs-4.9.1-1.el7.x86_64.rpm \
    https://rpmfind.net/linux/epel/7/x86_64/Packages/f/f2fs-tools-1.12.0-1.el7.x86_64.rpm \
    && yum clean all \
    && rpm -i http://mirror.centos.org/centos/8-stream/BaseOS/x86_64/os/Packages/libtraceevent-1.5.3-1.el8.x86_64.rpm --force \
    && rpm -i http://ftp.pasteur.fr/mirrors/CentOS/8-stream/PowerTools/x86_64/os/Packages/libtraceevent-devel-1.5.3-1.el8.x86_64.rpm

ADD run.sh /lkp-extent/run.sh
