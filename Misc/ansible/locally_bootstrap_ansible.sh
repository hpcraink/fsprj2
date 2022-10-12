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
sudo -u "${USERNAME}"  unzip -d /tmp /tmp/fsprj2-master.zip


# 3. Install playbook dependencies & run only the 'common' role
cd /tmp/fsprj2-master/Misc/ansible/

sudo -u "${USERNAME}"  ansible-galaxy install -r requirements.yml

# NOTE: Must be run as root since we don't know the become-pw (and we also don't want to ask (may be confusing to the user))
ansible localhost   -e hostname="${HOSTNAME}" -e username="${USERNAME}"  -m include_role -a name=roles/common


# 4. Fix fs permissions & ownership of repo  (despite `become: false`, we run ansbile role as root --> hence permissions are wrong)
chown -R "${USERNAME}:" "/home/${USERNAME}/fsprj2"
sudo -u "${USERNAME}"  git -C "/home/${USERNAME}/fsprj2" reset --hard origin/master



echo "Finished."
exit 0
