// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the EE1DLLTEST_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// EE1DLLTEST_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef EE1DLLTEST_EXPORTS
#define EE1DLLTEST_API __declspec(dllexport)
#else
#define EE1DLLTEST_API __declspec(dllimport)
#endif

// This class is exported from the dll
class EE1DLLTEST_API CEE1DLLTest {
public:
	CEE1DLLTest(void);
	// TODO: add your methods here.
};

extern EE1DLLTEST_API int nEE1DLLTest;

EE1DLLTEST_API int fnEE1DLLTest(void);
