#include <stdlib.h>
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>

#include <TestArray.h>
#include <TestBitArray.h>
#include <TestSparseArray.h>
#include <TestSet.h>
#include <TestMap.h>
#include <TestRingQueue.h>
#include <complex.h>
#include <DirectXMath.h>
using namespace DirectX;

int main()
{
	TestArray();
	TestBitArray();
	TestSparseArray();
	TestSet();
	TestMap();
	TestRingQueue();

	XMFLOAT4	float4(1.f, 1.f, 1.f, 1.f);
	auto Begin = std::chrono::system_clock::now();
	for (int i = 0; i < 1000'0000; ++i)
	{
		float4.x *= float4.x;
		float4.y *= float4.y;
		float4.z *= float4.z;
		float4.w *= float4.w;
	}
	auto End = std::chrono::system_clock::now();
	std::cout << "FPU time : " << std::chrono::duration<double, std::milli>(End - Begin).count() << " ms" << std::endl;
	std::cout << float4.x << std::endl;

	Begin = std::chrono::system_clock::now();
	XMVECTOR vec = XMLoadFloat4(&float4);
	for (int i = 0; i < 1000'0000; ++i)
	{
		vec = XMVectorMultiply(vec, vec);
		XMStoreFloat4(&float4, vec);
	}
	End = std::chrono::system_clock::now();
	std::cout << "SSE time : " << std::chrono::duration<double, std::milli>(End - Begin).count() << " ms" << std::endl;
	// XMStoreFloat4(&float4, vec);
	std::cout << float4.x << std::endl;

	system("pause");
	return 0;
}