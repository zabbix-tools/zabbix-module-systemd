# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  config.vm.box = "centos/7"
  config.vm.network "private_network", type: "dhcp"
  config.vm.synced_folder ".", "/vagrant", type: "nfs"
  config.vm.network "forwarded_port", guest: 10050, host: 10050
  config.vm.provision "shell", inline: <<-SHELL
    set -x

    # disable selinux
    setenforce 0
    cat > /etc/selinux/config <<EOL
SELINUX=permissive
SELINUXTYPE=targeted
EOL

    # configure systemd accounting
    cat > /etc/systemd/system.conf <<EOL
[Manager]
DefaultCPUAccounting=yes
DefaultBlockIOAccounting=yes
DefaultMemoryAccounting=yes
EOL
    systemctl daemon-reexec

    # install packages
    rpm -q zabbix-release > /dev/null 2>&1 || \
      rpm -i http://repo.zabbix.com/zabbix/3.2/rhel/7/x86_64/zabbix-release-3.2-1.el7.noarch.rpm
    yum install -y epel-release
    yum makecache
    yum update -y
    yum install -y \
      autoconf \
      automake \
      dbus-devel \
      gdb \
      libtool \
      policycoreutils-python \
      python2-pip \
      rpm-build \
      setools-console \
      strace \
      vim-enhanced \
      zabbix-agent \
      zabbix-get

    pip install zabbix-template-converter
    
    curl -sLO https://sourceforge.net/projects/zabbixagentbench/files/linux/zabbix_agent_bench-0.4.0.x86_64.tar.gz
    tar -xzf zabbix_agent_bench-0.4.0.x86_64.tar.gz
    install -m 0755 \
      zabbix_agent_bench-0.4.0.x86_64/zabbix_agent_bench \
      /usr/bin/zabbix_agent_bench
    
    # enable zabbix
    systemctl enable zabbix-agent
    systemctl start zabbix-agent
  SHELL
end
