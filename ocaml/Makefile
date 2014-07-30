all:
	swig -ocaml libppp.i
	swig -ocaml -co swig.mli ; swig -ocaml -co swig.ml
	ocamlc -c swig.mli ; ocamlc -c swig.ml
	ocamlc -c -ccopt "-I/usr/include/foo" libppp_wrap.c
	ocamlc -c libppp.mli
	ocamlc -c libppp.ml
	ld -shared libppp_wrap.o ../libppp.a -o libppp.so

clean:
	rm -f libppp.ml libppp.mli libppp.so libppp_wrap.c libppp_wrap.o swig* *.cmi *.cmo

flip:
	ocamlopt -o flip flip.ml

