{ pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/81f05d871faf75d1456df6adec1d2118d787f65c.tar.gz") {} }:

pkgs.stdenv.mkDerivation {
	name = "riscv-gnu-toolchain";
	src = /nix/store/z1v1d9bb1xv5c4l6lvlagb1h9zsbd18d-riscv-gnu-toolchain.tar.gz;
	buildInputs = with pkgs; [ autoconf automake curl python3 libmpc mpfr gmp gawk bison flex texinfo gperf libtool patchutils bc zlib expat binutils fakeroot file findutils gettext gnugrep groff gzip gnum4 gnumake pacman gnupatch pkg-config gnused sudo which flock perl ];
	dontBuild = true;
	installTargets = "newlib linux";
	installFlags = [ "--jobs=16" ];
	hardeningDisable = [ "format" ];
}
