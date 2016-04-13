//#ifdef DLLPROJECT_EXPORTS
#define TESTFUNCDLL_API __declspec(dllexport)
//#else
//#define TESTFUNCDLL_API __declspec(dllimport) 
//#endif

#include <string>

extern "C" {

	TESTFUNCDLL_API void mocapBind(int port);
	TESTFUNCDLL_API bool mocapBound();

	TESTFUNCDLL_API int markers();
	TESTFUNCDLL_API int segments();

	TESTFUNCDLL_API char* ThreadInfo();

	TESTFUNCDLL_API char * getSegment(int id, float &tx, float &ty, float &tz, float &rx, float &ry, float &rz, float &rw);

}

