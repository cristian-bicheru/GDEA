compile:
	
	g++ -I/usr/local/lib/cmake/symengine/../../../include -I/usr/include/x86_64-linux-gnu gdea.cpp -o test.bin -lgmp -rdynamic /usr/local/lib/x86_64-linux-gnu/libsymengine.a /usr/lib/x86_64-linux-gnu/libgmp.so -Wl,-rpath,/usr/lib/x86_64-linux-gnu -std=c++11
run:
	./test.bin
go:
	make compile
	make run
