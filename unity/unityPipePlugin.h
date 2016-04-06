//#ifdef DLLPROJECT_EXPORTS
#define TESTFUNCDLL_API __declspec(dllexport) 
//#else
//#define TESTFUNCDLL_API __declspec(dllimport) 
//#endif

extern "C" {

	TESTFUNCDLL_API int pointSize();
	TESTFUNCDLL_API int vertsSize();
	TESTFUNCDLL_API int getData(float *points, int *verts);


	TESTFUNCDLL_API void AddPosition(char *name, float tx, float ty, float tz, float rx, float ry, float rz, float rw);
	TESTFUNCDLL_API bool  Send();
	TESTFUNCDLL_API void  Clear();
	TESTFUNCDLL_API void  Close();

	TESTFUNCDLL_API char* ThreadInfo();
	TESTFUNCDLL_API char* PointInfo();

}
