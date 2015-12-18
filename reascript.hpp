#include "utilfuncs.h"
#include <unordered_set>
#ifdef WIN32
#include <ppl.h>
#endif
struct in { // Convenience struct to cast void** to supported parameter types
	void* v;

	in(void* const& v) : v(v) {}

	operator double() { return *(double*)v; }
	operator double*() { return (double*)v; }

	operator int() { return (int)(INT_PTR)v; }
	operator int*() { return (int*)(INT_PTR)&v; }

	operator bool() { return *(bool*)v; }
	operator bool*() { return (bool*)v; }

	operator char() { return *(char*)v; }
	operator char*() { return (char*)v; }
	operator const char*() { return (const char*)v; }
};

// These macros helps in dealing with the function definition and supported return types. 
// Use return_int to return anything that is not a double. Alternatively, use obj for class/struct.
#define params void** arg, int arg_sz
#define return_double(v) { double* lhs_arg = (double*)(arg[arg_sz-1]); *lhs_arg = (v); return (void*)lhs_arg; }
#define return_int(v) return (void*)(INT_PTR)(v)
#define return_null return (void*)0
#define return_obj(c) return (void*)c

// At the moment (REAPER v5pre6) the supported parameter types are:
//  - int, int*, bool, bool*, double, double*, char*, const char*
//  - AnyStructOrClass* (handled as an opaque pointer)
// At the moment (REAPER v5pre6) the supported return types are:
//  - int, bool, double, const char*
//  - AnyStructOrClass* (handled as an opaque pointer)

function_entry MRP_DoublePointer("double", "double,double", "n1,n2", [](params) {
	double* n1 = (in)arg[0];
	double* n2 = (in)arg[1];

	return_double(*n1 + *n2);
},
"add two numbers"
);

function_entry MRP_IntPointer("int", "int,int", "n1,n2", [](params) {
	int* n1 = (in)arg[0];
	int* n2 = (in)arg[1];

	return_int(*n1+*n2);
},
"add two numbers"
);

std::unordered_set<void*> g_active_mrp_arrays;

function_entry MRP_CreateArray("MRP_Array*", "int", "size", [](params) {
	char buf[256];
	int* arrsize = (in)arg[0];
	std::vector<double>* ret;
	try
	{
		ret = new std::vector<double>(*arrsize);
		g_active_mrp_arrays.insert((void*)ret);
		return (void*)ret;
	}
	catch (std::exception& ex)
	{
		sprintf(buf, "MRP_CreateArray : failed to create array (%s)", ex.what());
		ReaScriptError(buf);
	}
	return_null;
},
"Create an array of 64 bit floating point numbers. Note that these will leak memory if they are not later destroyed with MRP_DestroyArray!"
);

function_entry MRP_DestroyArray("", "MRP_Array*", "array", [](params) {
	std::vector<double>* vecptr = (std::vector<double>*)arg[0];
	if (g_active_mrp_arrays.count(arg[0]) == 0)
	{
		ReaScriptError("Script tried passing invalid MRP_Array for destruction");
		return (void*)nullptr;
	}
	if (vecptr != nullptr)
	{
		delete vecptr;
		g_active_mrp_arrays.erase(arg[0]);
	}
	return (void*)nullptr;
},
"Destroy a previously created MRP_Array"
);

function_entry MRP_GenerateSine("", "MRP_Array*,double,double", "array,samplerate,frequency", [](params) {
	if (g_active_mrp_arrays.count(arg[0]) == 0)
	{
		ReaScriptError("MRP_GenerateSine : passed in invalid MRP_Array");
		return (void*)nullptr;
	}
	std::vector<double>& vecref = *(std::vector<double>*)arg[0];
	double sr = bound_value(1.0,*(double*)arg[1],1000000.0);
	double hz = bound_value(0.0001,*(double*)arg[2],sr/2.0);
	int numsamples = vecref.size();
	for (size_t i = 0; i < numsamples; ++i)
		vecref[i] = sin(2*3.141592653/sr*hz*i);
	return (void*)nullptr;
},
"Generate a sine wave into a MRP_Array"
);

function_entry MRP_MultiplyArrays("", "MRP_Array*,MRP_Array*", "array1, array2", [](params) {
	if (g_active_mrp_arrays.count(arg[0]) == 0 || g_active_mrp_arrays.count(arg[1]) == 0)
	{
		ReaScriptError("MRP_MultiplyArrays : passed in invalid MRP_Array(s)");
		return (void*)nullptr;
	}
	std::vector<double>& vecref0 = *(std::vector<double>*)arg[0];
	std::vector<double>& vecref1 = *(std::vector<double>*)arg[1];
	if (vecref0.size() != vecref1.size())
	{
		ReaScriptError("MRP_MultiplyArrays : incompatible array lengths");
		return (void*)nullptr;
	}
	for (size_t i = 0; i < vecref0.size(); ++i)
		vecref0[i] = vecref0[i] * vecref1[i];
	return (void*)nullptr;
},
"Multiply 2 MRP_Arrays of same length. First array is overwritten with result!"
);
#ifdef WIN32
function_entry MRP_MultiplyArraysMT("", "MRP_Array*,MRP_Array*", "array1, array2", [](params) {
	if (g_active_mrp_arrays.count(arg[0]) == 0 || g_active_mrp_arrays.count(arg[1]) == 0)
	{
		ReaScriptError("MRP_MultiplyArraysMT : passed in invalid MRP_Array(s)");
		return (void*)nullptr;
	}
	std::vector<double>& vecref0 = *(std::vector<double>*)arg[0];
	std::vector<double>& vecref1 = *(std::vector<double>*)arg[1];
	if (vecref0.size() != vecref1.size())
	{
		ReaScriptError("MRP_MultiplyArraysMT : incompatible array lengths");
		return (void*)nullptr;
	}
	Concurrency::parallel_for((size_t)0, vecref0.size(), [&vecref0, &vecref1](size_t index) 
	{
		vecref0[index] = vecref0[index] * vecref1[index];
	},Concurrency::static_partitioner());
	return (void*)nullptr;
},
"Multiply 2 MRP_Arrays of same length. First array is overwritten with result! Uses multiple threads."
);
#else
function_entry MRP_MultiplyArraysMT("", "MRP_Array*,MRP_Array*", "array1, array2", [](params)
{
	MRP_MultiplyArrays(params);
}
#endif
function_entry MRP_SetArrayValue("", "MRP_Array*,int,double", "array, index, value", [](params) {
	std::vector<double>& vecref0 = *(std::vector<double>*)arg[0];
	int index = (in)arg[1];
	double v = *(double*)arg[2];
	vecref0[index] = v;
	return (void*)nullptr;
},
"Set MRP_Array element value. No safety checks done for array or index validity, so use at your own peril!"
);

function_entry MRP_GetArrayValue("double", "MRP_Array*,int", "array, index", [](params) {
	std::vector<double>& vecref0 = *(std::vector<double>*)arg[0];
	int index = (in)arg[1];
	double value = vecref0[index];
	return_double(value);
},
"Get MRP_Array element value. No safety checks done for array or index validity, so use at your own peril!"
);

function_entry MRP_WriteArrayToFile("", "MRP_Array*,const char*,double", "array,filename,samplerate", [](params) {
	if (g_active_mrp_arrays.count(arg[0]) == 0)
	{
		ReaScriptError("MRP_WriteArrayToFile : passed in invalid MRP_Array");
		return (void*)nullptr;
	}
	std::vector<double>& vecref = *(std::vector<double>*)arg[0];
	const char* outfn = (const char*)arg[1];
	double sr = bound_value(1.0, *(double*)arg[2], 1000000.0);
	char cfg[] = { 'e','v','a','w', 32, 0 };
	PCM_sink* sink = PCM_Sink_Create(outfn, cfg, sizeof(cfg), 1, sr, false);
	if (sink != nullptr)
	{
		double* sinkbuf[1];
		sinkbuf[0] = vecref.data();
		sink->WriteDoubles(sinkbuf, vecref.size(), 1, 0, 1);
		delete sink;
	} else
		ReaScriptError("MRP_WriteArrayToFile : could not create output file");
	return (void*)nullptr;
},
"Write MRP_Array to disk as a 32 bit floating point mono wav file"
);

function_entry MRP_CalculateEnvelopeHash("int", "TrackEnvelope*", "env", [](params) 
{
	TrackEnvelope* env = (TrackEnvelope*)arg[0];
	if (env == nullptr)
		return_null;
	int numpoints = CountEnvelopePoints(env);
	size_t seed = 0;
	for (int i = 0; i < numpoints; ++i) {
		double pt_time = 0.0;
		double pt_val = 0.0;
		double pt_tension = 0.0;
		int pt_shape = 0;
		GetEnvelopePoint(env, i, &pt_time, &pt_val, &pt_shape, &pt_tension, nullptr);
		hash_combine(seed, pt_time);
		hash_combine(seed, pt_val);
		hash_combine(seed, pt_tension);
		hash_combine(seed, pt_shape);
	}
	return_int((int)seed);
},
"This <i>function</i> isn't really <b>correct...</b> it calculates a 64 bit hash "
"but returns it as a 32 bit int. Should reimplement this. "
"Or rather, even more confusingly : The hash will be 32 bit when building "
"for 32 bit architecture and 64 bit when building for 64 bit architecture! "
"It comes down to how size_t is of different size between the 32 and 64 bit "
"architectures."
);

function_entry MRP_ReturnMediaItem("MediaItem*", "", "", [](params) {
	return_obj(GetSelectedMediaItem(0, 0));
},
"return media item"
);

function_entry MRP_DoNothing("", "", "", [](params) {
	return_null;
},
"do nothing, return null"
);

function_entry MRP_DoublePointerAsInt("int", "double,double", "n1,n2", [](params) {
	double* n1 = (in)arg[0];
	double* n2 = (in)arg[1];

	return_double(*n1 + *n2);
},
"add two numbers"
);

function_entry MRP_CastDoubleToInt("int", "double,double", "n1,n2", [](params) { // This is for demonstration purposes.
	int n1 = *(double*)arg[0];                                                     // I don't know why you'd cast the type.
	int n2 = *(double*)arg[1];                                                     // Instead, just use the right input types.

	return_int(n1+n2);
},
"add two numbers"
);

function_entry MRP_CastIntToDouble("double", "int,int", "n1,n2", [](params) {
	double n1 = *(int*)arg[0];
	double n2 = *(int*)arg[1];

	return_int(n1 + n2);
},
"add two numbers"
);

#undef lambda
#undef return_double
#undef return_int
#undef return_void