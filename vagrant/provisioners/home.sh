#!/bin/bash

if ! grep -qF "cd /vagrant" /home/vagrant/.bashrc ; then
  echo "cd /vagrant" >> /home/vagrant/.bashrc
fi

chown vagrant:vagrant /home/vagrant/.bashrc