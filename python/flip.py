import libppp

state = libppp.pp_new_state()
print("> state created")

libppp.pp_load_file(state, "../parse/models/flip.model")
print("> file loaded")

instance = libppp.pp_new_instance(state, "flip_example", None)
print("> instance created")

query = libppp.pp_compile_query(instance, "x>2, x<3")
print("> condition compiled")

traces = libppp.pp_sample(instance, query)
print("> traces sampled")

query = libppp.pp_compile_query(instance, "flip==1")
print("> query compiled")

success, result = libppp.pp_get_result(traces, query)
print(result)

success, result = libppp.pp_infer(instance, "flip == 1", "x > 2, x < 3")
print(result)

libppp.pp_free(state); 
