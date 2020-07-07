#include <stdlib.h>
#include <Containers/Map.h>

int main()
{
	Fuko::TMap<int, int> Map;
	Map.Reserve(100);
	Fuko::TMultiMap<int, int> MutiMap;

	Map[1] = 3;

	system("pause");
	return 0;
}