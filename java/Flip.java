
public class Flip {
    static {
	System.loadLibrary("ppp");
    }
    public static void main(String[] args) {
        // System.setProperty("java.library.path", ".");
	// System.loadLibrary("libppp");
        SWIGTYPE_p_pp_state_t state = libppp.pp_new_state();
        SWIGTYPE_p_pp_instance_t instance;
        SWIGTYPE_p_pp_query_t query;
        SWIGTYPE_p_pp_trace_store_t traces;
        int success;
	float[] result = new float[2];

        System.out.print("> state created\n");
    
        libppp.pp_load_file(state, "../parse/models/flip.model");
        System.out.print("> file loaded\n");
    
        instance = libppp.pp_new_instance(state, "flip_example", null);
        System.out.print("> instance created\n");
    
        query = libppp.pp_compile_query(instance, "x>2, x<3");
        System.out.print("> condition compiled\n");
    
        traces = libppp.pp_sample(instance, query);
        System.out.print("> traces sampled\n");
    
        query = libppp.pp_compile_query(instance, "flip==1");
        System.out.print("> query compiled\n");
    
        success = libppp.pp_get_result(traces, query, result);
        System.out.printf("result: %f\n", result[0]);
    
        success = libppp.pp_infer(instance, "flip == 1", "x > 2, x < 3", result);
        System.out.printf("result: %f\n", result[0]);
    
        libppp.pp_free(state);
    }
 
}
