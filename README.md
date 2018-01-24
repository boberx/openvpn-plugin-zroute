# openvpn-plugin-zroute
# Description
An OpenVPN plugin for managing Zebra static routes  
(tested with Debian 9 on x86 and quagga 1.2.2)  
# How to build
* Setting up a chroot

```sh
apt-get install coreutils bash debootstrap
```

```sh
ARCH="i386"; SUITE="stretch"; CHROOTDIR="/storage/"${SUITE}"-chroot_"${ARCH}""; LC="ru_RU.UTF-8";
mkdir -p "${CHROOTDIR}" && \
debootstrap --arch="${ARCH}" --variant=minbase --include=locales,apt-utils,dialog,findutils,file,sed,gawk,bzip2 \
"${SUITE}" "${CHROOTDIR}" http://mirror.mephi.ru/debian/ && \
echo "LANG="${LC}"" > "${CHROOTDIR}"/etc/default/locale && \
sed -i 's/# '"${LC}"' UTF-8/'"${LC}"' UTF-8/' "${CHROOTDIR}"/etc/locale.gen && \
chmod 777 "${CHROOTDIR}"/home && \
chroot "${CHROOTDIR}" /bin/bash -c "su - -c \"locale-gen\"";
```
* Install the required packages in a chroot environment

```sh
apt-get --no-install-recommends install make gcc libc6-dev:i386 openvpn libquagga-dev
```

* compile

```sh
make
```

# How to use
Set the plugin in the OpenVPN configfile  
> /etc/openvpn/openvpn.conf
  
```
plugin /usr/lib/openvpn/openvpn-plugin-zroute.so
```

Change socket permissions  
```
chmod 766 /run/quagga/zserv.api
```
# For testing

Add route using vtysh  
```sh
vtysh -c "configure terminal" -c "ip route 10.30.3.8 255.255.255.255 tun0"
```