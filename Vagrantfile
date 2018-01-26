# -*- mode: ruby -*-
# vi: set ft=ruby :

require File.dirname(__FILE__) + "/vagrant/addons/dependency_manager"

check_plugins ["vagrant-exec", "vagrant-hostmanager", "vagrant-triggers"]

Vagrant.configure("2") do |config|
  # All Vagrant configuration is done here. The most common configuration
  # options are documented and commented below. For a complete reference,
  # please see the online documentation at vagrantup.com.

  # Every Vagrant virtual environment requires a box to build off of.
  config.vm.box = "precise64"

  # set ip address to 192.168.90.09
  config.vm.network "private_network", ip: "192.168.90.09", hostsupdater: "skip"

  # set hostname
  config.vm.hostname = "guildmud.dev"

  # setup hostmanager
  config.hostmanager.enabled = true
  config.hostmanager.manage_host = true
  config.hostmanager.manage_guest = true
  config.hostmanager.ignore_private_ip = false
  config.hostmanager.include_offline = true

  # install packages
  config.vm.provision "shell", path: "vagrant/provisioners/home.sh"
  config.vm.provision "shell", path: "vagrant/provisioners/c.sh"
  config.vm.provision "shell", path: "vagrant/provisioners/sqlite.sh"

  # make /vagrant the working directory for all commands
  config.exec.commands '*', directory: '/vagrant'
end
