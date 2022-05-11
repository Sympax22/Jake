{ pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/81f05d871faf75d1456df6adec1d2118d787f65c.tar.gz") {} }:

pkgs.stdenv.mkDerivation {
  name = "man-jake";
  src = ../../man;
  buildInputs = with pkgs; [ pandoc man ];
  builder = ./builder.sh;
}
