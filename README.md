# easystroke
Fork of thjaeger / easystroke with all of the latest patches

You can use the following instructions to install on a Debian-based distribution:

```
wget http://ftp.ubuntu.com/ubuntu/pool/universe/e/easystroke/easystroke_0.6.0-0ubuntu11.debian.tar.xz
wget http://ftp.ubuntu.com/ubuntu/pool/universe/e/easystroke/easystroke_0.6.0-0ubuntu11.dsc
sudo apt-get install -y devscripts
sudo mk-build-deps -i easystroke_0.6.0-0ubuntu11.dsc
wget https://github.com/higersky/easystroke/archive/master.tar.gz
tar zxf master.tar.gz
cd easystroke-master
make
sudo make install
```
