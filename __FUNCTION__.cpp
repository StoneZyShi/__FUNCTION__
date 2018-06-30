#include <iostream>

using namespace std;

void main()
{
	cout << __DATE__ << endl;
	cout << __TIME__ << endl;
	cout << __FILE__ << endl;
	cout << __LINE__ << endl;
	cout << __TIMESTAMP__ << endl;
	cout << __FUNCTION__ << endl;

#ifdef WINDOWS_VS2005_UP
	cout << __FUNCSIG__ << endl;
	cout << __FUNCDNAME__ << endl;
#endif // WINDOWS



	getchar();
}
