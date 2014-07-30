all:
	swig -python libppp.i
	gcc -c libppp_wrap.c -I/usr/include/python2.7
	ld -shared libppp_wrap.o ../libppp.a -o _libppp.so

clean:
	rm -f libppp.py libppp.pyc _libppp.so libppp_wrap.c libppp_wrap.o
flip:
	python flip.py

