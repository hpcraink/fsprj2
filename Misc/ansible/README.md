# Setup dev environment for libiotrace (via ansible)
## Prerequisites
* Downloaded [Ubuntu](https://ubuntu.com/download/desktop) (or any other Debian-based distro) image
* Installed Ansible (on Mac: `brew install ansible`)
* Installed playbook dependencies: `ansible-galaxy install -r requirements.yml`


## Steps
DISCLAIMER: Has been tested atm only on Mac using VMware Fusion

### Locally managed (via shell)
1. **Install Ubuntu**
2. **Run** the **Ansible Role** locally: `wget -q -O - https://raw.githubusercontent.com/hpcraink/fsprj2/master/Misc/ansible/locally_bootstrap_ansible.sh | sudo bash`
3. Reboot system (recommended (the new membership in the group `docker` won't have no effect otherwise))

### Remotely managed (via ssh)
1. **Install Ubuntu** as VM (e.g. on Mac, in [VirutalBox](https://www.virtualbox.org/))
    * During installation (for Ubuntu) tick *Install third-party software for graphics ans Wi-Fi hardware [...]* and use:
      * username: `hse`
      * password: `1`
      * hostname: `hse-dev-vm`
    * After installation:
      * Install ssh: `sudo apt install -y ssh`
      * Find out the IP-address of the VM: `ip -4 a` (must be run OBVIOUSLY inside the VM)
2. **Generate** a new **SSH key**: `ssh-keygen -t ed25519 -C hse-host -f ~/.ssh/hse-dev-vm`  (press Enter to skip passphrase prompt)
3. **Copy ssh key** to VM: `ssh-copy-id -i ~/.ssh/hse-dev-vm.pub hse@<IP-ADDRESS-OF-VM>`
4. **Add** following (!! MUST BE ADAPTED !!) **entry** to `~/.ssh/config`:
    ```
    Host hse-dev-vm
      HostName <IP-ADDRESS-OF-VM>
      Port 22
      IdentityFile ~/.ssh/hse-dev-vm
      User hse
    ```
5. **Run playbook**: `ansible-playbook run.yml`
6. Reboot system (optional, but recommended)

# Setup dev environment for libiotrace (manually on Windows)

## Prerequisites
* Downloaded [Ubuntu](https://ubuntu.com/download/desktop) (or any other Debian-based distro) image
* Downloaded [Docker Desktop](https://docs.docker.com/desktop/install/windows-install/
)

## Steps
DISCLAIMER: You need a computer with Hyper-V Virtualization support (e.g. Windows 10, not Windows10 Home)

### Via shell
1. **Enable WSL2** : dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
2. **Enable Virtual Machine Platform**: 
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart
3. **Set WSL2 as default**: wsl --set-default-version 2

### Via Ubuntu
4. **Convert Ubuntu from WSL1 to WSL2**:  wsl.exe --set-version Ubuntu 2

### Via Docker Desktop
5. **Go on Settings/General**: "Use the WSL2 based engine" has to be checked
6. **Go on Settings/Resources/WSL Integration**: Enable integartion on Ubuntu