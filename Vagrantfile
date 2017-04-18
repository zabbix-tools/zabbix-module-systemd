# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  config.vm.box = "centos/7"
  config.vm.network "private_network", type: "dhcp"
  config.vm.synced_folder ".", "/vagrant", type: "nfs"
  config.vm.provision "shell", inline: <<-SHELL
    set -x
    yum update -y
    rpm -q zabbix-release > /dev/null 2>&1 || \
      rpm -i http://repo.zabbix.com/zabbix/3.2/rhel/7/x86_64/zabbix-release-3.2-1.el7.noarch.rpm
    yum install -y \
      autoconf \
      automake \
      dbus-devel \
      gdb \
      libtool \
      strace \
      vim-enhanced \
      zabbix-agent \
      zabbix-get
    
    for version in 3.2.4 3.0.8 2.4.8; do
      if [[ ! -f "zabbix-${version}.tar.gz" ]]; then
        curl -LO http://sourceforge.net/projects/zabbix/files/ZABBIX%20Latest%20Stable/${version}/zabbix-${version}.tar.gz
      fi

      if [[ ! -d "/usr/src/zabbix-${version}" ]]; then
        tar -xz -C /usr/src -f zabbix-${version}.tar.gz
      fi
    done
    ln -s /usr/src/zabbix-3.2.4/ /usr/src/zabbix
  SHELL
end
