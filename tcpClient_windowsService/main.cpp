
#include"tcpClient_windowsService.h"





int wmain(int argc, LPWSTR* argv)
{
#if WORK_AS_SERVICE
	WorkAsService(argc, argv);
#else
	int result = ServiseEssentialWork();
	system("pause");
#endif
	return 0;
}

