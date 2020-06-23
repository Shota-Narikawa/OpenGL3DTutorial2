/**
*@file Collision.cpp
*/
#include "Collision.h"
#include <algorithm>

namespace Collision {

	/**
	*���`����쐬����.
	*
	*@param center	���̒��S���W.
	*@param r		���̔��a.
	*
	*@return		����ێ�����ėp�Փˌ`��I�u�W�F�N�g.
	*/
	Shape CreateSphere(const glm::vec3& center, float r)
	{
		Shape result;
		result.type = Shape::Type::sphere;
		result.s = Sphere{ center, r };
		return result;
	}

	/**
	*�J�v�Z���`����쐬����.
	*
	*@param a	���S�̐����̎n�_���W.
	*@param b	���S�̐����̏I�_���W.
	*@param r	�J�v�Z���̔��a.
	*
	*@return	�J�v�Z����ێ�����ėp�Փˌ`��I�u�W�F�N�g.
	*/
	Shape CreateCapsule(const glm::vec3& a, const glm::vec3& b, float r) {
		Shape result;
		result.type = Shape::Type::capsule;
		result.c = Capsule{ { a, b }, r };
		return result;
	}

	/**
	*�L�����E�{�b�N�X�`����쐬����.
	*
	*@param center	�L�����E�{�b�N�X�̒��S���W.
	*@param axisX	X���̌���.
	*@param axisY	Y���̌���.
	*@param axisZ	Z���̌���.
	*@param e		XYZ�������̕�.
	*
	*@return		�L�����E�{�b�N�X��ێ�����ėp�Փˌ`��I�u�W�F�N�g.
	*/
	Shape CreateOBB(const glm::vec3& center, const glm::vec3& axisX,
		const glm::vec3& axisY, const glm::vec3& axisZ, const glm::vec3& e) {
		Shape result;
		result.type = Shape::Type::obb;
		result.obb = OrientedBoundingBox{ center,
			{ normalize(axisX), normalize(axisY), normalize(axisZ) }, e };
		return result;

	}

	/**
	*���Ƌ����Փ˂��Ă��邩���ׂ�.
	*
	*@param s0		����Ώۂ̋����̂P.
	*@param s1		����Ώۂ̋����̂Q.
	*
	*@retval true	�Փ˂��Ă���.
	*@retval false	�Փ˂��ĂȂ�.
	*/
	bool TestSphereSphere(const Sphere& s0, const Sphere& s1)
	{
		const glm::vec3 m = s0.center - s1.center;
		const float radiusSum = s0.r + s1.r;
		return glm::dot(m, m) <= radiusSum * radiusSum;
	}

	/**
	*�����Ɠ_�̍ŋߐړ_�𒲂ׂ�.
	*
	*@param seg	����.
	*@param p	�_.
	*
	*@return seg��p�̍ŋߐړ_.
	*/
	glm::vec3 ClosestPointSegment(const Segment& seg, const glm::vec3& p)
	{
		const glm::vec3 ab = seg.b - seg.a;
		const glm::vec3 ap = p - seg.a;
		const float lenAQ = glm::dot(ab, ap);
		const float lenAB = glm::dot(ab, ab);
		if (lenAQ <= 0) {
			return seg.a;

		}
		else if (lenAQ >= lenAB) {
			return seg.b;

		}
		return seg.a + ab * (lenAQ / lenAB);
	}

	/**
	*���ƃJ�v�Z�����Փ˂��Ă��邩���ׂ�.
	*
	*@param s		��.
	*@param c		�J�v�Z��.
	*@param p		�ŋߐړ_�̊i�[��.
	*
	*@retval true	�Փ˂��Ă���.
	*@retval false	�Փ˂��Ă��Ȃ�.
	*/
	bool TestSphereCapsule(const Sphere& s, const Capsule& c, glm::vec3* p)
	{
		*p = ClosestPointSegment(c.seg, s.center);
		const glm::vec3 distance = *p - s.center;
		const float radiusSum = s.r + c.r;
		return glm::dot(distance, distance) <= radiusSum * radiusSum;
	}

	/**
	*OBB�Ɠ_�̍ŋߐړ_�𒲂ׂ�.
	*
	*@param obb		�L�����E�{�b�N�X.
	*@param p		�_.
	*
	*@return obb��p�̍ŋߐړ_.
	*/
	glm::vec3 ClosestPointOBB(const OrientedBoundingBox& obb, const glm::vec3& p)
	{
		const glm::vec3 d = p - obb.center;
		glm::vec3 q = obb.center;
		for (int i = 0; i < 3; ++i) {
			float distance = dot(d, obb.axis[i]);
			if (distance >= obb.e[i]) {
				distance = obb.e[i];

			}
			else if (distance <= -obb.e[i]) {
				distance = -obb.e[i];
			}
			q += distance * obb.axis[i];
		}
		return q;
	}

	/**
	*����OBB���Փ˂��Ă��邩���ׂ�.
	*
	*@param s		��.
	*@param obb		�L�����E�{�b�N�X.
	*@param p		�ŋߐړ_�̊i�[��.
	*
	*@retval true	�Փ˂��Ă���.
	*@retval false	�Փ˂��Ă��Ȃ�.
	*/
	bool TestSphereOBB(const Sphere& s, const OrientedBoundingBox& obb, glm::vec3* p)
	{
		*p = ClosestPointOBB(obb, s.center);
		const glm::vec3 distance = *p - s.center;
		return dot(distance, distance) <= s.r * s.r;
	}

/**
* 2�̐����̍ŋߐړ_�𒲂ׂ�.
*
*@param		s0	����A.
*@param		s1	����B.
*@param		c0	����A��̍ŋߐړ_.
*@param		c1	����B��̍ŋߐړ_.
*
*@return	c0,	c1�Ԃ̂̒�����2��.
*/
float ClosestPoint(const Segment& s0, const Segment& s1, glm::vec3& c0, glm::vec3& c1)
{
		//�����̎n�_�ƕ����x�N�g����ϐ��ɑ��.
		const glm::vec3 Pa = s0.a;
		const glm::vec3 Da = s0.b - s0.a;
		const glm::vec3 Pb = s1.a;
		const glm::vec3 Db = s1.b - s1.a;
		
		//���̋��ʕ������v�Z���ĕϐ��ɑ��.
		const float a = glm::dot(Da, Da);
		const float b = glm::dot(Da, Db);
		const float c = glm::dot(Da, Pa - Pb);
		const float e = glm::dot(Db, Db);
		const float f = glm::dot(Db, Pa - Pb);

		//�����̍ŋߐړ_�v�Z�Ɏg������.
		const float denom = a * e - b * b;
		
		//���������s�łȂ��Ȃ�(denom��0�ȊO)�A�����̍ŋߐړ_�����߂鎮�ŉ���s���v�Z.
		//���s�ȏꍇ�͐���A��̓K���Ȉʒu(�Ƃ肠����0)��I��.
		float s = 0;
		if(denom != 0.0f)
		{
			s = (b * f - c * e) / denom;
			s = glm::clamp(s, 0.0f, 1.0f);
		}

		//����B��̍ŋߐړ_�����߂鎮��t���v�Z.
			float t = (b * s + f) / e;
			
			//t��0��菬�����A�܂���1���傫���ꍇ�At��0�`1�ɐ������A
			//����A��̍ŋߐړ_�����߂鎮��s���Čv�Z. t��0�`1�̏ꍇ�͌v�Z����.
			if (t < 0){
			t = 0;
			//s = (tb - c) / a���t = 0�Ȃ̂�s = -c / a�ƂȂ�
			s = glm::clamp(-c / a, 0.0f, 1.0f);
		}
			else if (t > 1)
			{
			t = 1;
			// s = (tb - c) / a���t = 1�Ȃ̂�s = (b - c) / a�ƂȂ�
			s = glm::clamp((b - c) / a, 0.0f, 1.0f);
		}

			//�v�Z����s, t����ŋߐړ_�̍��W���v�Z.
			c0 = Pa + Da * s;
			c1 = Pb + Db * t;
				//�ŋߐړ_�̋�����2���Ԃ�.
				return glm::dot(c0 - c1, c0 - c1);
}

/**
*�J�v�Z���ƃJ�v�Z�����Փ˂��Ă��邩���ׂ�.
*
*@param		c0	�J�v�Z��0.
*@param		c1	�J�v�Z��1.
*
*@return �Փˌ��ʂ��i�[����Result�^�̒l.
*/
Result TestCapsuleCapsule(const Capsule& c0, const Capsule& c1)
{
	// �J�v�Z�������̐������m�̋������v�Z���A���̋�����2�̃J�v�Z���̔��a�̘a�ȉ��Ȃ�Փ˂��Ă���.
		glm::vec3 p0, p1;
	const float d = ClosestPoint(c0.seg, c1.seg, p0, p1);
	const float r = c0.r + c1.r;
	if (d > r * r) {
		return {};
	}
	
	Result result;
	result.isHit = true;
	result.na = glm::normalize(p1 - p0);
	result.nb = -result.na;
	result.pa = p0 + result.na * c0.r;
	result.pb = p1 + result.nb * c1.r;
	return result;
}

/**
*�����Ǝ����s���E�{�b�N�X���������Ă��邩���ׂ�.
*
*@param		p	�����̎n�_.
*@param		d		�����̕����x�N�g��.
*@param		aabb	�����s���E�{�b�N�X.
*@param		tmin	�����������i�[����ϐ�.
*@param		q		�����_���i�[����ϐ�.
*
*@retval	true	�������Ă���.
*@retval	false	�������Ă��Ȃ�.
*/
bool IntersectRayAABB(const glm::vec3& p, const glm::vec3& d, const AABB& aabb,
	float& tmin, glm::vec3& q)
{
	tmin = 0; // �ł��������˓_�܂ł̋���.
	float tmax = FLT_MAX; // �ł��߂��o�˓_�܂ł̋���.
	
	for (int i = 0; i < 3; ++i)
	{
		// �����Ǝ������s���ǂ���.
		if (std::abs(d[i]) < FLT_EPSILON)
		{
		// ���s�ȏꍇ�A�����̎n�_�����̕��ʂ̊O���ɂ�����������Ȃ�.
			if (p[i] < aabb.min[i] || p[i] > aabb.max[i])
			{
			return false;
			}
		}
		else
		{
		//�n�_������˓_����яo�˓_�܂ł̋������v�Z.
		const float ood = 1.0f / d[i]; // ���Z�̉񐔂����炷.
		float t1 = (aabb.min[i] - p[i]) * ood;
		float t2 = (aabb.max[i] - p[i]) * ood;

			//�߂��ق�����˓_�A�����ق����o�˓_�Ƃ���.
			if (t1 > t2) 
			{
			std::swap(t1, t2);
			}
			//�n�_����ł��������˓_�܂ł̋����A�ł��߂��o�˓_�܂ł̋������X�V.
			tmin = std::max(tmin, t1);
			tmax = std::min(tmax, t2);
		
			//���˓_���o�˓_��艓���Ȃ���������Ȃ�.
			if (tmin > tmax)
			{
			return false;
			}
		}
	}
	//������AABB�ƌ������Ă���. �����_q���v�Z����.
	q = p + d * tmin;
	return true;
}

/**
*AABB�̒��_���擾����.
*
*@param		aabb	AABB.
*@param		n		���_��I������r�b�g�t���O.
*
*@return	n�ɑΉ����钸�_���W.
*/
glm::vec3 Corner(const AABB& aabb, int n)
{
	glm::vec3 p = aabb.min;
	if (n & 1) { p.x = aabb.max.x; }
	if (n & 2) { p.y = aabb.max.y; }
	if (n & 4) { p.z = aabb.max.z; }
	return p;
}

/**
*�J�v�Z����AABB���Փ˂��Ă��邩���ׂ�.
*
*@param		c		�J�v�Z��.
*@param		aabb	�����s���E�{�b�N�X.
*
*@return			�Փˌ��ʂ��i�[����Result�^�̒l.
*/
Result TestCapsuleAABB(const Capsule& c, const AABB& aabb)
{
	//�J�v�Z���̔��a�����g�債��AABB���쐬.
	AABB e = aabb;
	e.min -= c.r;
	e.max += c.r;

		//�����̕����x�N�g�����v�Z. 0 <= t < 1�Ő����̔�����ł���悤�ɂ��邽�߁A���K���͂��Ȃ�.
		const glm::vec3 d(c.seg.b - c.seg.a);
		//�g�債��AABB�ƌ����̌�������.
		float t;
		glm::vec3 p;
	if (!IntersectRayAABB(c.seg.a, d, e, t, p) || t > 1) {
		return {};
	}
	//�����̋N�������̈��ϐ�u(������), v(������)�ɋL�^.
		int u = 0, v = 0;
	if (p.x < aabb.min.x) { u |= 1; }
	else if (p.x > aabb.max.x) { v |= 1; }
	
		if (p.y < aabb.min.y) { u |= 2; }
	else if (p.y > aabb.max.y) { v |= 2; }
	
		if (p.z < aabb.min.z) { u |= 4; }
	else if (p.z > aabb.max.z) { v |= 4; }
	
		const int m = u + v;

		//m = 0�Ȃ�΁A��_p��AABB�̓����ɂ���.
			//p���ł��߂��ʂɎˉe���A���̓_���Փ˓_�Ƃ���.
			if (!m) {
			Result result;
			result.isHit = true;
			
			result.pb = glm::vec3(aabb.min.x, p.y, p.z);
			result.nb = glm::vec3(-1, 0, 0);
			float d = p.x - aabb.min.x; // �ł��߂��ʂ܂ł̋���.
			if (aabb.max.x - p.x < d) {
				d = aabb.max.x - p.x;
				result.pb = glm::vec3(aabb.max.x, p.y, p.z);
				result.nb = glm::vec3(1, 0, 0);
			}
			if (p.y - aabb.min.y < d) {
				d = p.y - aabb.min.y;
				result.pb = glm::vec3(p.x, aabb.min.y, p.z);
				result.nb = glm::vec3(0, -1, 0);
			}
			if (aabb.max.y - p.y < d) {
				d = aabb.max.y - p.y;
				result.pb = glm::vec3(p.x, aabb.max.y, p.z);
				result.nb = glm::vec3(0, 1, 0);
			}
			if (p.z - aabb.min.z < d) {
				d = p.z - aabb.min.z;
				result.pb = glm::vec3(p.x, p.y, aabb.min.z);
				result.nb = glm::vec3(0, 0, -1);
			}
			if (aabb.max.z - p.z < d) {
				d = aabb.max.z - p.z;
				result.pb = glm::vec3(p.x, p.y, aabb.max.z);
				result.nb = glm::vec3(0, 0, 1);
			}
			result.na = -result.nb;
			result.pa = p + result.na * c.r;
			return result;
		}
			//p�͖ʗ̈�ɂ���.
				if ((m & (m - 1)) == 0) {
				Result result;
				result.isHit = true;

					//���̎��_��p�͊g�����ꂽAABB��ɂ���.
					//�@���̋t�����Ɉړ������_�����ۂ̏Փ˓_�Ƃ���.
					if (u & 1) {
					result.nb = glm::vec3(-1, 0, 0);
					result.pb = p + result.nb * (p.x - aabb.min.x);
				}
					else if (u & 2) {
					result.nb = glm::vec3(0, -1, 0);
					result.pb = p + result.nb * (p.y - aabb.min.y);
				}
					else if (u & 4) {
					result.nb = glm::vec3(0, 0, -1);
					result.pb = p + result.nb * (p.z - aabb.min.z);
				}
					else if (v & 1) {
					result.nb = glm::vec3(1, 0, 0);
					result.pb = p - result.nb * (p.x - aabb.max.x);
				}
					else if (v & 2) {
					result.nb = glm::vec3(0, 1, 0);
					result.pb = p - result.nb * (p.y - aabb.max.y);
				}
					else if (v & 4) {
					result.nb = glm::vec3(0, 0, 1);
					result.pb = p - result.nb * (p.z - aabb.max.z);
				}
				result.na = -result.nb;
				result.pa = p + result.na * c.r;
				return result;
			}
				//p�͒��_�̈�ɂ���.
					//���_�ɐڂ���3�ӂ̂����A�ł��ڋ߂����ŋߐړ_�����ӂƍŏ��ɏՓ˂����Ƃ݂Ȃ�.
					if (m == 7) {
					const glm::vec3 bv = Corner(aabb, v);
					glm::vec3 c0, c1, c2, c3;
					float d = ClosestPoint(c.seg, Segment{ bv, Corner(aabb, v ^ 1) }, c0, c1);
					float d0 = ClosestPoint(c.seg, Segment{ bv, Corner(aabb, v ^ 2) }, c2, c3);
					if (d0 < d) {
						d = d0;
						c0 = c2;
						c1 = c3;
					}
					d0 = ClosestPoint(c.seg, Segment{ bv, Corner(aabb, v ^ 4) }, c2, c3);
					if (d0 < d) {
						d = d0;
						c0 = c2;
						c1 = c3;
					}
					if (d > c.r * c.r) {
						return {};
					}
					Result result;
					result.isHit = true;
					result.na = glm::normalize(c1 - c0);
					result.pa = c0 + result.na * c.r;
					result.pb = c1;
					return result;
				}
					//p�͕ӗ̈�ɂ���.
						{
						glm::vec3 c0, c1;
						const Segment edge = { Corner(aabb, u ^ 7), Corner(aabb, v) };
						const float d = ClosestPoint(c.seg, edge, c0, c1);
						if (d > c.r * c.r) {
							return {};
						}
						Result result;
						result.isHit = true;
						result.na = glm::normalize(c1 - c0);
						result.pa = c0 + result.na * c.r;
						result.pb = c1;
						result.nb = -result.na;
						return result;
						}
}

/**
*�J�v�Z����OBB���Փ˂��Ă��邩���ׂ�.
*
*@param	c	�J�v�Z��.
*@param	obb	�L�����E�{�b�N�X.
*
*@return	�Փˌ��ʂ��i�[����Result�^�̒l.
*/
Result TestCapsuleOBB(const Capsule& c, const OrientedBoundingBox& obb)
{
	//������OBB�̃��[�J�����W�n�ɕϊ�.
	Capsule cc = c;
	cc.seg.a -= obb.center;
	cc.seg.b -= obb.center;
	const glm::mat3 matOBB(
	glm::transpose(glm::mat3(obb.axis[0], obb.axis[1], obb.axis[2])));
	cc.seg.a = matOBB * cc.seg.a;
	cc.seg.b = matOBB * cc.seg.b;

		//�Փ˔���.
		Result result = TestCapsuleAABB(cc, AABB{ -obb.e, obb.e });
	if (result.isHit) {
		//�Փˌ��ʂ����[���h���W�n�ɕϊ�.
		const glm::mat3 matInvOBB(glm::inverse(matOBB));
		result.pa = matInvOBB * result.pa + obb.center;
		result.pb = matInvOBB * result.pb + obb.center;
		result.na = matInvOBB * result.na;
		result.nb = matInvOBB * result.nb;
	}
	return result;
}

/**
*���Ɖ��炩�̌`��̏Փ˔���.
*
*@param		a	����Ώۂ̃V�F�C�v���̂P.
*@param		b	����Ώۂ̃V�F�C�v���̂Q.
*
*@return	�Փˌ��ʂ��i�[����Result�^�̒l.
*/
Result TestSphereShape(const Sphere& a, const Shape& b)
{
	Result result;
	glm::vec3 p;
	switch (b.type) {
	case Shape::Type::sphere:
		if (TestSphereSphere(a, b.s)) {
			result.isHit = true;
			result.na = glm::normalize(b.s.center - a.center);
			result.nb = -result.na;
			result.pa = a.center + result.na * a.r;
			result.pb = b.s.center + result.nb * b.s.r;
		}
		break;
	case Shape::Type::capsule:
		if (TestSphereCapsule(a, b.c, &p)) {
			result.isHit = true;
			result.na = glm::normalize(p - a.center);
			result.nb = -result.na;
			result.pa = a.center + result.na * a.r;
			result.pb = p + result.nb * b.c.r;
		}
		break;
	case Shape::Type::obb:
		if (TestSphereOBB(a, b.obb, &p)) {
			result.isHit = true;
			result.na = glm::normalize(p - a.center);
			result.nb = -result.na;
			result.pa = a.center + result.na * a.r;
			result.pb = p;
		}
		break;
	}
	return result;
}

/**
*�J�v�Z���Ɖ��炩�̌`��̏Փ˔���.
*
*@param		a	����Ώۂ̃V�F�C�v���̂P.
*@param		b	����Ώۂ̃V�F�C�v���̂Q.
*
*@return	�Փˌ��ʂ��i�[����Result�^�̒l.
*/
Result TestCapsuleShape(const Capsule& a, const Shape& b)
{
	glm::vec3 p;
	switch (b.type) {
	case Shape::Type::sphere:
		if (TestSphereCapsule(b.s, a, &p)) {
			Result result;
			result.isHit = true;
			result.na = glm::normalize(b.s.center - p);
			result.nb = -result.na;
			result.pa = p + result.na * a.r;
			result.pb = b.s.center + result.nb * b.s.r;
			return result;
		}
		break;

	case Shape::Type::capsule:
		return TestCapsuleCapsule(a, b.c);
		
	case Shape::Type::obb:
		return TestCapsuleOBB(a, b.obb);
	}
	return {};
}

/**
*OBB�Ɖ��炩�̌`��̏Փ˔���.
*
*@param	a	����Ώۂ̃V�F�C�v���̂P.
*@param		b	����Ώۂ̃V�F�C�v���̂Q.
*
*@return	�Փˌ��ʂ��i�[����Result�^�̒l.
*/
Result TestOBBShape(const OrientedBoundingBox& a, const Shape& b)
{
	Result result;
	glm::vec3 p;
	switch (b.type) {
	case Shape::Type::sphere:
		if (TestSphereOBB(b.s, a, &p)) {
			result.isHit = true;
			result.na = glm::normalize(b.s.center - p);
			result.nb = -result.na;
			result.pa = p;
			result.pb = b.s.center + result.nb * b.s.r;
		}
		break;
		
	case Shape::Type::capsule:
		result = TestCapsuleOBB(b.c, a);
		std::swap(result.na, result.nb);
		std::swap(result.pa, result.pb);
		break;
	
	case Shape::Type::obb:
		//������.
			break;
	}
	return result;
}

/**
*�V�F�C�v���m���Փ˂��Ă��邩���ׂ�.
*
*@param		a	����Ώۂ̃V�F�C�v���̂P.
*@param		b	����Ώۂ̃V�F�C�v���̂Q.
*
*@return	�Փˌ��ʂ�\��Result�^�̒l.
*/
Result TestShapeShape(const Shape& a, const Shape& b)
{
	switch (a.type) {
	case Shape::Type::sphere:
		return TestSphereShape(a.s, b);
		
	case Shape::Type::capsule:
		return TestCapsuleShape(a.c, b);
		
	case Shape::Type::obb:
		return TestOBBShape(a.obb, b);
	}
	return Result{};
}

	/**
	*�V�F�C�v���m���Փ˂��Ă��邩���ׂ�.
	*
	*@param a	����Ώۂ̃V�F�C�v���̂P.
	*@param b	����Ώۂ̃V�F�C�v���̂Q.
	*@param pa	�Փ˂������W.
	*@param pb	�Փ˂������W.
	*
	*@retval true	�Փ˂���.
	*@retval false	�Փ˂��Ȃ�����.
	*/
	bool TestShapeShape(const Shape& a, const Shape& b, glm::vec3* pa, glm::vec3* pb)
	{
		if (a.type == Shape::Type::sphere) {
			if (b.type == Shape::Type::sphere) {
				if (TestSphereSphere(a.s, b.s)) {
					*pa = a.s.center;
					*pb = b.s.center;
					return true;

				}
			}
			else if (b.type == Shape::Type::capsule) {
				if (TestSphereCapsule(a.s, b.c, pb)) {
					*pa = a.s.center;
					return true;
				}
			}
			else if (b.type == Shape::Type::obb) {
				if (TestSphereOBB(a.s, b.obb, pb)) {
					*pa = a.s.center;
					return true;
				}
			}
		}
		else if (a.type == Shape::Type::capsule) {
			if (b.type == Shape::Type::sphere) {
				if (TestSphereCapsule(b.s, a.c, pa)) {
					*pb = b.s.center;
					return true;

				}
			}
		}
		else if (a.type == Shape::Type::obb) {
			if (b.type == Shape::Type::sphere) {
				if (TestSphereOBB(b.s, a.obb, pa)) {
					*pb = b.s.center;
					return true;

				}
			}
		}
		return false;
	}

} // namespace Collision