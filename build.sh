if [ ! -d $(pwd)/bin ]; then
    mkdir $(pwd)/bin
fi

rm -rf $(pwd)/bin/*

if [ ! -d $(pwd)/build ]; then
    mkdir $(pwd)/build
fi

rm -rf $(pwd)/build/*

if [ ! -d $(pwd)/lib ]; then
    mkdir $(pwd)/lib
fi

rm -rf $(pwd)/lib/*


cd $(pwd)/build &&
    cmake .. &&
    make

