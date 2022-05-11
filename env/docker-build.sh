sudo docker build -t computer-engineering/jake .
sudo docker save computer-engineering/jake > jake.env.tar
gzip -9 < jake.env.tar > jake.env.tar.gz
