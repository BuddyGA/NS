#include "nsUnitTest.h"
#include "nsFileSystem.h"



int main(int argc, char* argv[])
{
	nsUnitTest::TestArray();
	nsUnitTest::TestString();
	nsUnitTest::TestMath();

	const nsString file = "C:/Users/Buddy/AppData/Local/Temp.txt";
	
	const nsString path = nsFileSystem::FileGetPath(file);
	NS_Validate(path == "C:/Users/Buddy/AppData/Local");

	const nsName name = nsFileSystem::FileGetName(file);
	NS_Validate(name == "Temp");

	const nsName ext = nsFileSystem::FileGetExtension(file);
	NS_Validate(ext == ".txt");

	return 0;
}
