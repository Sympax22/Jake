let
  pkgs = import (fetchTarball
    "https://github.com/NixOS/nixpkgs/archive/528d35bec0cb976a06cc0e8487c6e5136400b16b.tar.gz")
    { };
  riscv-gnu-toolchain = import ../riscv-gnu-toolchain { };
  RISCV = riscv-gnu-toolchain.outPath;
in pkgs.stdenv.mkDerivation rec {
  name = "riscv-pk";
  src = fetchTarball
    "https://github.com/riscv-software-src/riscv-pk/archive/fb77b0c20020773a36b7fe2660cf02a89fedbeb9.tar.gz";

  inherit RISCV;

  buildInputs = [ riscv-gnu-toolchain ];

  preConfigure = ''
    mkdir build
    cd build
  '';

  configureScript = "../configure";

  makeFlags = ''
    CC=${RISCV}/bin/riscv64-unknown-elf-gcc
    AR=${RISCV}/bin/riscv64-unknown-elf-ar
    OBJCOPY=${RISCV}/bin/riscv64-unknown-elf-objcopy
  '';
}
