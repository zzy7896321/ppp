open Swig;;
open Libppp;;

let flip_eg =
  let state = _pp_new_state(C_void) in

  _pp_load_file(C_list [state, C_string "../parse/models/flip.model"]);

  let instance = _pp_new_instance(state, "flip_example", []) in

  let query = _pp_compile_query(instance, "x>2, x<3") in

  let traces = _pp_sample(instance, query) in

  let query = _pp_compile_query(instance, "flip==1") in

  let (success, result) = _pp_get_result(traces, query) in

  let (success, result) = _pp_infer(instance, "flip == 1", "x > 2, x < 3") in
  
  _pp_free(state); 
