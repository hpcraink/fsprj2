#!/usr/bin/env bash

#######################################################################
# Runs Ansible script directly locally on the machine via the shell
# Removes therefore the need of managing the machine remotely (via ssh)
#######################################################################

set -e

USERNAME="$(logname)"
HOSTNAME="$(hostname)"


# 0. Check whether requirements are fulfilled
if [ "$EUID" -ne 0 ]; then
  echo "Please run this script as root"
  exit 1
fi

source /etc/os-release
if [[ "$NAME" != "Ubuntu" ]]; then
  echo "Only Ubuntu is currently supported"
  exit 1
fi


# 1. Install Ansible
apt-add-repository ppa:ansible/ansible
apt update
apt install -y ansible


# 2. Download & extract repo
sudo -u "${USERNAME}"  wget -O /tmp/fsprj2-master.zip https://github.com/hpcraink/fsprj2/archive/refs/heads/master.zip
sudo -u "${USERNAME}"  unzip -o -d /tmp /tmp/fsprj2-master.zip


# 3. Install playbook dependencies & run only the 'common' role
cd /tmp/fsprj2-master/Misc/ansible/

sudo -u "${USERNAME}"  ansible-galaxy install -r requirements.yml

# NOTE: Must be run as correct user, otherwise fs ownership will be wrong  (but requires us 2 ask for the root pw)
# ALSO: We can't startup the docker containers b/c the group membership will only be updated once we log in again (and using the root user instead causes permission issues for Grafana plugins)
#  --> Has to be done manually by the user via `docker-compose up -d` once logged in again
echo -e "\nPlease enter your root pw once again"
sudo -u "${USERNAME}"  ansible localhost  --ask-become-pass   -e hostname="${HOSTNAME}" -e username="${USERNAME}" -e libiotrace_setup_docker_containers=false  -m include_role -a name=roles/common



echo "Finished. Pls reboot now."
exit 0
