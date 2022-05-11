set -e

pkgs="https://github.com/NixOS/nixpkgs/archive/81f05d871faf75d1456df6adec1d2118d787f65c.tar.gz"
url="https://polybox.ethz.ch/index.php/s/RVbtFxGMPVpaHkb/download"
hash="0xrs3fi4hg89gs6w6hv5yjdgxs2zz23d7kg49nnbnh5kfisp93il"
deps="man-db-2.9.4 git-2.34.1 gnumake-4.3 spike-1.0.0 python3-3.10.1 vim-8.2.3877"
localdeps="riscv-gnu-toolchain riscv-pk qemu"
manpages="man-jake"
root=$(dirname $(realpath $BASH_SOURCE))

optimise() {
	nix-store --optimise
}

__install_localdep() {
	nix-env -f $root/$1 -i $1
}

install_manpages() {
	nix-env --no-filter-syscalls -f $root/$manpages -i $manpages
}

install() {
	nix-prefetch-url $url $hash --print-path --name "riscv-gnu-toolchain.tar.gz"
	for localdep in $localdeps; do __install_localdep $localdep; done
	nix-env -f $pkgs -i $deps
}

uninstall() {
	nix-env -e $manpages $localdeps $deps
}


case $1 in
	--uninstall)
		uninstall
		optimise
		;;
	--manpages)
		install_manpages
		;;
	*)
		install
		optimise
		;;
esac
