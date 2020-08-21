#include <Gfx.h>
#include <iostream>

int main(void)
{
	using namespace Fuko::Gfx;
	IGfxSystem* System = GetGfxSystem();
	GfxSystemDesc Desc;
	Desc.EnableDebugLayer = true;
	System->Init(Desc);
	
	std::wcout << System->GetAdapter(0)->Name() << std::endl;
	std::wcout << System->GetAdapter(1)->Name() << std::endl;

	{
		System->CreateDevice(System->GetAdapter(0));
	}

	System->ShutDown();
	system("pause");
	return 0;
}