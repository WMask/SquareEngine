/***************************************************************************
* HelloWorld.cpp
*/

#include "Core/SMath.h"
#include "Core/SUtils.h"
#include <iostream>


int main()
{
	std::cout << "Hello World!\n";
	std::cout << "Squre Engine Version: " << SGetEngineVersion();

	SMatrix4 m4 = SConst::IdentitySMatrix4;
	m4.m[1] = 2.5f;
	std::cout << "\n\nMat:\n" << Eigen::Matrix4<float>(m4.m) << std::endl << std::endl;

	m4 = SMath::TransposeM4(m4);
	std::cout << "\nMat transpose:\n" << Eigen::Matrix4<float>(m4.m) << std::endl;

	SPoint2 p1{10, 10};
	SPoint2 p2{2, 2};

	if (p1 == p2)
	{
		p1 = p1 + 2;
	}
	else
	{
		p2 = p1 / 2;
	}

	return 0;
}
