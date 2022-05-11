let
	pkgs = import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/528d35bec0cb976a06cc0e8487c6e5136400b16b.tar.gz") {};
in pkgs.qemu.override { hostCpuTargets = ["riscv64-softmmu"]; }
