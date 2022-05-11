source $stdenv/setup

cp $src/* .
make
mkdir -p $out
if [ ! -d ./share/man ]; then mkdir -p ./share/man; fi
mandb ./share/man
mv ./share $out
