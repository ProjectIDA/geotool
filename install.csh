#!/bin/csh -f
gtar xzf cbase.tar.gz
gtar xzf gbase.tar.gz
gtar xzf geotool.tar.gz
gtar xzf gsl-1.16.tar.gz

set install_dir = /usr/local/geotool

cd gsl-1.16
./configure --prefix=$install_dir
make
make install

cd ../cbase
autoreconf --verbose --force --install
./configure CC=gcc CXX=g++ --prefix=$install_dir --with-pic
make
make check
make install

cd ../gbase
./bootstrap
./configure CC=gcc CXX=g++ CFLAGS="-g -fPIC -DENVIRONMENT"  CXXFLAGS="-g -fPIC" \
		CPPFLAGS="-I$install_dir/include -I$install_dir/include/libCD" \
		LDFLAGS="-L$install_dir/lib -L$install_dir/lib" \
		--prefix=$install_dir
make
make check
make install

cd ../geotool
./bootstrap
./configure CC=gcc CXX=g++ CFLAGS="-g -fPIC -DENVIRONMENT"  CXXFLAGS="-g -fPIC"\
		CPPFLAGS="-I$install_dir/include -I$install_dir/include -I$install_dir/include -I$install_dir/" \
		LDFLAGS="-L$install_dir/lib -L$install_dir/lib -L$install_dir/lib" \
		--prefix=$install_dir
make
make check
make install
