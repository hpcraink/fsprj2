# Setup dev environment for libiotrace
## Prerequisites
* Downloaded [Ubuntu](https://ubuntu.com/download/desktop) (or any other Debian-based distro) image
* Installed Ansible (on Mac: `brew install ansible`)
* Installed playbook dependencies: `ansible-galaxy install -r requirements.yml`


## Steps  (NOTE: tested only on Mac)
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
