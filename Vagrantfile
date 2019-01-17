# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.require_version ">= 2.0.0"

Vagrant.configure("2") do |config|
  config.vm.box = "bento/ubuntu-18.04"
  config.vm.box_check_update = false
  config.vm.synced_folder "./", "/workspace"
  config.vm.provision "shell", inline: <<-SHELL
    sudo apt-get update
    sudo DEBIAN_FRONTEND=noninteractive apt-get install -y \
        arping                                             \
        build-essential                                    \
        clang-format                                       \
        cmake                                              \
        libboost-all-dev                                   \
        ninja-build                                        \
        python3                                            \
        python3-pip                                        \
        tshark                                             \
        tmux                                               \
        unzip                                              \
        wget

    sudo pip3 install meson

    cd /tmp &&                                                                 \
        wget https://github.com/abseil/googletest/archive/release-1.8.1.zip && \
        unzip release-1.8.1.zip &&                                             \
        cd googletest-release-1.8.1 &&                                         \
        mkdir build &&                                                         \
        cd build &&                                                            \
        cmake .. &&                                                            \
        make install

    cd /tmp &&                                                                 \
        wget https://github.com/google/benchmark/archive/v1.4.1.zip &&         \
        unzip v1.4.1.zip &&                                                    \
        cd benchmark-1.4.1 &&                                                  \
        mkdir build &&                                                         \
        cd build &&                                                            \
        cmake .. &&                                                            \
        make install

    echo "cd /workspace" >> /home/vagrant/.bashrc
    echo "/workspace/scripts/tap.sh" >> /home/vagrant/.bashrc
  SHELL
  config.vm.network "private_network", ip: "10.255.255.101"
  config.vm.provider "virtualbox" do |vb|
    vb.cpus = "4"
    vb.memory = "4096"
    vb.customize ["modifyvm", :id, "--nicpromisc1", "allow-all"]
    vb.customize ["modifyvm", :id, "--nicpromisc2", "allow-all"]
  end
end
