all:
	swig -java libppp.i && gcc -c libppp_wrap.c -I/usr/lib/jvm/java-7-openjdk-i386/include && ld -shared libppp_wrap.o ../libppp.a -o libppp.so

clean:
	rm -f Flip.class libppp.java libppp.class libpppJNI.* SWIGTYPE*  libppp.so libppp_wrap.c libppp_wrap.o

flip:
	javac Flip.java
	java Flip

