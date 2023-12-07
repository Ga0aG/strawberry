# Tutorial for cmake

## A. Hello world

1. Go to the folder of A-hello-world
1. Build the code `cmake .. -S . -B build -DBUILD_TESTING=ON`
1. Test the code `make test -C build`
1. Install targets `make install -C build`
