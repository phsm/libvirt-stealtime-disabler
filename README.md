# Libvirt kvm-steal-time disabler
Qemu allows disabling KVM steal time by passing a special `-kvm-steal-time` CPU flag. Libvirt does not recognize this flag, so there is no way to set it in the libvirt.

This library is meant to be used as LD_PRELOAD hook to libvirtd process to wrap the stdlib `execve()` function. 
It detects when libvirt tries to launch "qemu-system-*" process, and injects the cpu flag into it, effectively disabling steal time accounting for the virtual machine.

This is an alternative to patching libvirt/qemu/kernel.

# How to use
1. Build it by running `make`
2. Put the resulting .so file into some directory.
3. Create a systemd drop-in file in the following directory: `/etc/systemd/system/<the name of libvirt service>.d/injectflag.conf`. You can find the name of the libvirt service by running `systemctl | grep libvirt`. In Debian, Ubuntu systems it is `libvirtd.service`.

The drop-in file contents:
```
[Service]
Environment=LD_PRELOAD=/<your>/<directory>/injectflag.so
```

4. Daemon reload and restart libvirtd:
```
systemctl daemon-reload
systemctl restart libvirtd
```

5. Try to start a virtual machine and look if the resulting qemu command line contains it: `ps aux | grep qemu | grep steal-time`. If its there, then you are good to go.
