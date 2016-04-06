//#ifdef DLLPROJECT_EXPORTS
#define TESTFUNCDLL_API __declspec(dllexport) 
//#else
//#define TESTFUNCDLL_API __declspec(dllimport) 
//#endif

extern "C" {

	TESTFUNCDLL_API void mocapBind(int port);
	TESTFUNCDLL_API bool mocapBound();

	TESTFUNCDLL_API int markers();
	TESTFUNCDLL_API int segments();

	TESTFUNCDLL_API char* ThreadInfo();

}

