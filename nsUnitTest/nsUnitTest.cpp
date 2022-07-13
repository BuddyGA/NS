#include "nsUnitTest.h"
#include "nsFileSystem.h"



int main(int argc, char* argv[])
{
	nsUnitTest::TestArray();
	nsUnitTest::TestString();
	nsUnitTest::TestMath();

	const nsString file = TEXT("C:/Users/Buddy/AppData/Local/Temp.txt");
	
	const nsString path = nsFileSystem::FileGetPath(file);
	NS_Validate(path == TEXT("C:/Users/Buddy/AppData/Local"));

	const nsString name = nsFileSystem::FileGetName(file);
	NS_Validate(name == TEXT("Temp"));

	const nsString ext = nsFileSystem::FileGetExtension(file);
	NS_Validate(ext == TEXT(".txt"));

	return 0;
}
