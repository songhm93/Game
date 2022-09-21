#include "pch.h"
#include "Frustum.h"
#include "Camera.h"

void Frustum::FinalUpdate()
{
	Matrix matViewInv = Camera::S_MatView.Invert();
	Matrix matProjectionInv = Camera::S_MatProjection.Invert();
	Matrix matInv = matProjectionInv * matViewInv; //world 변환 행렬

	vector<Vec3> worldPos =
	{    // 시계 방향. -1~1은 투영좌표계에 해당하는 정점들임. 위에서 구한 world변환 행렬로 world좌표로 변환한다.
		::XMVector3TransformCoord(Vec3(-1.f, 1.f, 0.f), matInv),
		::XMVector3TransformCoord(Vec3(1.f, 1.f, 0.f), matInv),
		::XMVector3TransformCoord(Vec3(1.f, -1.f, 0.f), matInv),
		::XMVector3TransformCoord(Vec3(-1.f, -1.f, 0.f), matInv),
		::XMVector3TransformCoord(Vec3(-1.f, 1.f, 1.f), matInv),
		::XMVector3TransformCoord(Vec3(1.f, 1.f, 1.f), matInv),
		::XMVector3TransformCoord(Vec3(1.f, -1.f, 1.f), matInv),
		::XMVector3TransformCoord(Vec3(-1.f, -1.f, 1.f), matInv)
	};

	//평면을 만들어준다.
	_planes[PLANE_FRONT] = ::XMPlaneFromPoints(worldPos[0], worldPos[1], worldPos[2]); // CW 시계방향
	_planes[PLANE_BACK] = ::XMPlaneFromPoints(worldPos[4], worldPos[7], worldPos[5]); // CCW 반시계반향
	_planes[PLANE_UP] = ::XMPlaneFromPoints(worldPos[4], worldPos[5], worldPos[1]); // CW
	_planes[PLANE_DOWN] = ::XMPlaneFromPoints(worldPos[7], worldPos[3], worldPos[6]); // CCW
	_planes[PLANE_LEFT] = ::XMPlaneFromPoints(worldPos[4], worldPos[0], worldPos[7]); // CW 
	_planes[PLANE_RIGHT] = ::XMPlaneFromPoints(worldPos[5], worldPos[6], worldPos[1]); // CCW 얘네가 어떻게 기준으로 시계방향 반시계 방향인지가 불분명하다;
}

bool Frustum::ContainsSphere(const Vec3& pos, float radius) //pos는 물체의 원점, 물체의 범위를 구로 표현. 물체가 절두체안에 있는지 체크
{
	for (const Vec4& plane : _planes)
	{
		// n = (a, b, c)
		Vec3 normal = Vec3(plane.x, plane.y, plane.z);

		// (ax + by + cz) + d > radius
		if (normal.Dot(pos) + plane.w > radius)
			return false;
	}

	return true;
}