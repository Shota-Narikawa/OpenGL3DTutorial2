/**
*@file Collision.cpp
*/
#include "Collision.h"
#include <algorithm>

namespace Collision {

	/**
	*球形状を作成する.
	*
	*@param center	球の中心座標.
	*@param r		球の半径.
	*
	*@return		球を保持する汎用衝突形状オブジェクト.
	*/
	Shape CreateSphere(const glm::vec3& center, float r)
	{
		Shape result;
		result.type = Shape::Type::sphere;
		result.s = Sphere{ center, r };
		return result;
	}

	/**
	*カプセル形状を作成する.
	*
	*@param a	中心の線分の始点座標.
	*@param b	中心の線分の終点座標.
	*@param r	カプセルの半径.
	*
	*@return	カプセルを保持する汎用衝突形状オブジェクト.
	*/
	Shape CreateCapsule(const glm::vec3& a, const glm::vec3& b, float r) {
		Shape result;
		result.type = Shape::Type::capsule;
		result.c = Capsule{ { a, b }, r };
		return result;
	}

	/**
	*有向境界ボックス形状を作成する.
	*
	*@param center	有向境界ボックスの中心座標.
	*@param axisX	X軸の向き.
	*@param axisY	Y軸の向き.
	*@param axisZ	Z軸の向き.
	*@param e		XYZ軸方向の幅.
	*
	*@return		有向境界ボックスを保持する汎用衝突形状オブジェクト.
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
	*球と球が衝突しているか調べる.
	*
	*@param s0		判定対象の球その１.
	*@param s1		判定対象の球その２.
	*
	*@retval true	衝突している.
	*@retval false	衝突してない.
	*/
	bool TestSphereSphere(const Sphere& s0, const Sphere& s1)
	{
		const glm::vec3 m = s0.center - s1.center;
		const float radiusSum = s0.r + s1.r;
		return glm::dot(m, m) <= radiusSum * radiusSum;
	}

	/**
	*線分と点の最近接点を調べる.
	*
	*@param seg	線分.
	*@param p	点.
	*
	*@return segとpの最近接点.
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
	*球とカプセルが衝突しているか調べる.
	*
	*@param s		球.
	*@param c		カプセル.
	*@param p		最近接点の格納先.
	*
	*@retval true	衝突している.
	*@retval false	衝突していない.
	*/
	bool TestSphereCapsule(const Sphere& s, const Capsule& c, glm::vec3* p)
	{
		*p = ClosestPointSegment(c.seg, s.center);
		const glm::vec3 distance = *p - s.center;
		const float radiusSum = s.r + c.r;
		return glm::dot(distance, distance) <= radiusSum * radiusSum;
	}

	/**
	*OBBと点の最近接点を調べる.
	*
	*@param obb		有向境界ボックス.
	*@param p		点.
	*
	*@return obbとpの最近接点.
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
	*球とOBBが衝突しているか調べる.
	*
	*@param s		球.
	*@param obb		有向境界ボックス.
	*@param p		最近接点の格納先.
	*
	*@retval true	衝突している.
	*@retval false	衝突していない.
	*/
	bool TestSphereOBB(const Sphere& s, const OrientedBoundingBox& obb, glm::vec3* p)
	{
		*p = ClosestPointOBB(obb, s.center);
		const glm::vec3 distance = *p - s.center;
		return dot(distance, distance) <= s.r * s.r;
	}

/**
* 2つの線分の最近接点を調べる.
*
*@param		s0	線分A.
*@param		s1	線分B.
*@param		c0	線分A上の最近接点.
*@param		c1	線分B上の最近接点.
*
*@return	c0,	c1間のの長さの2乗.
*/
float ClosestPoint(const Segment& s0, const Segment& s1, glm::vec3& c0, glm::vec3& c1)
{
		//線分の始点と方向ベクトルを変数に代入.
		const glm::vec3 Pa = s0.a;
		const glm::vec3 Da = s0.b - s0.a;
		const glm::vec3 Pb = s1.a;
		const glm::vec3 Db = s1.b - s1.a;
		
		//式の共通部分を計算して変数に代入.
		const float a = glm::dot(Da, Da);
		const float b = glm::dot(Da, Db);
		const float c = glm::dot(Da, Pa - Pb);
		const float e = glm::dot(Db, Db);
		const float f = glm::dot(Db, Pa - Pb);

		//直線の最近接点計算に使う除数.
		const float denom = a * e - b * b;
		
		//線分が平行でないなら(denomが0以外)、直線の最近接点を求める式で仮のsを計算.
		//平行な場合は線分A上の適当な位置(とりあえず0)を選択.
		float s = 0;
		if(denom != 0.0f)
		{
			s = (b * f - c * e) / denom;
			s = glm::clamp(s, 0.0f, 1.0f);
		}

		//線分B上の最近接点を求める式でtを計算.
			float t = (b * s + f) / e;
			
			//tが0より小さい、または1より大きい場合、tを0～1に制限し、
			//線分A上の最近接点を求める式でsを再計算. tが0～1の場合は計算完了.
			if (t < 0){
			t = 0;
			//s = (tb - c) / aよりt = 0なのでs = -c / aとなる
			s = glm::clamp(-c / a, 0.0f, 1.0f);
		}
			else if (t > 1)
			{
			t = 1;
			// s = (tb - c) / aよりt = 1なのでs = (b - c) / aとなる
			s = glm::clamp((b - c) / a, 0.0f, 1.0f);
		}

			//計算したs, tから最近接点の座標を計算.
			c0 = Pa + Da * s;
			c1 = Pb + Db * t;
				//最近接点の距離の2乗を返す.
				return glm::dot(c0 - c1, c0 - c1);
}

/**
*カプセルとカプセルが衝突しているか調べる.
*
*@param		c0	カプセル0.
*@param		c1	カプセル1.
*
*@return 衝突結果を格納するResult型の値.
*/
Result TestCapsuleCapsule(const Capsule& c0, const Capsule& c1)
{
	// カプセル内部の線分同士の距離を計算し、その距離が2つのカプセルの半径の和以下なら衝突している.
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
*光線と軸平行境界ボックスが交差しているか調べる.
*
*@param		p	光線の始点.
*@param		d		光線の方向ベクトル.
*@param		aabb	軸平行境界ボックス.
*@param		tmin	交差距離を格納する変数.
*@param		q		交差点を格納する変数.
*
*@retval	true	交差している.
*@retval	false	交差していない.
*/
bool IntersectRayAABB(const glm::vec3& p, const glm::vec3& d, const AABB& aabb,
	float& tmin, glm::vec3& q)
{
	tmin = 0; // 最も遠い入射点までの距離.
	float tmax = FLT_MAX; // 最も近い出射点までの距離.
	
	for (int i = 0; i < 3; ++i)
	{
		// 光線と軸が平行かどうか.
		if (std::abs(d[i]) < FLT_EPSILON)
		{
		// 平行な場合、光線の始点が軸の平面の外側にあったら交差なし.
			if (p[i] < aabb.min[i] || p[i] > aabb.max[i])
			{
			return false;
			}
		}
		else
		{
		//始点から入射点および出射点までの距離を計算.
		const float ood = 1.0f / d[i]; // 除算の回数を減らす.
		float t1 = (aabb.min[i] - p[i]) * ood;
		float t2 = (aabb.max[i] - p[i]) * ood;

			//近いほうを入射点、遠いほうを出射点とする.
			if (t1 > t2) 
			{
			std::swap(t1, t2);
			}
			//始点から最も遠い入射点までの距離、最も近い出射点までの距離を更新.
			tmin = std::max(tmin, t1);
			tmax = std::min(tmax, t2);
		
			//入射点が出射点より遠くなったら交差なし.
			if (tmin > tmax)
			{
			return false;
			}
		}
	}
	//光線はAABBと交差している. 交差点qを計算する.
	q = p + d * tmin;
	return true;
}

/**
*AABBの頂点を取得する.
*
*@param		aabb	AABB.
*@param		n		頂点を選択するビットフラグ.
*
*@return	nに対応する頂点座標.
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
*カプセルとAABBが衝突しているか調べる.
*
*@param		c		カプセル.
*@param		aabb	軸平行境界ボックス.
*
*@return			衝突結果を格納するResult型の値.
*/
Result TestCapsuleAABB(const Capsule& c, const AABB& aabb)
{
	//カプセルの半径だけ拡大したAABBを作成.
	AABB e = aabb;
	e.min -= c.r;
	e.max += c.r;

		//光線の方向ベクトルを計算. 0 <= t < 1で線分の判定をできるようにするため、正規化はしない.
		const glm::vec3 d(c.seg.b - c.seg.a);
		//拡大したAABBと光線の交差判定.
		float t;
		glm::vec3 p;
	if (!IntersectRayAABB(c.seg.a, d, e, t, p) || t > 1) {
		return {};
	}
	//交差の起こった領域を変数u(負方向), v(正方向)に記録.
		int u = 0, v = 0;
	if (p.x < aabb.min.x) { u |= 1; }
	else if (p.x > aabb.max.x) { v |= 1; }
	
		if (p.y < aabb.min.y) { u |= 2; }
	else if (p.y > aabb.max.y) { v |= 2; }
	
		if (p.z < aabb.min.z) { u |= 4; }
	else if (p.z > aabb.max.z) { v |= 4; }
	
		const int m = u + v;

		//m = 0ならば、交点pはAABBの内側にある.
			//pを最も近い面に射影し、その点を衝突点とする.
			if (!m) {
			Result result;
			result.isHit = true;
			
			result.pb = glm::vec3(aabb.min.x, p.y, p.z);
			result.nb = glm::vec3(-1, 0, 0);
			float d = p.x - aabb.min.x; // 最も近い面までの距離.
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
			//pは面領域にある.
				if ((m & (m - 1)) == 0) {
				Result result;
				result.isHit = true;

					//この時点でpは拡張されたAABB上にある.
					//法線の逆方向に移動した点を実際の衝突点とする.
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
				//pは頂点領域にある.
					//頂点に接する3辺のうち、最も接近した最近接点を持つ辺と最初に衝突したとみなす.
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
					//pは辺領域にある.
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
*カプセルとOBBが衝突しているか調べる.
*
*@param	c	カプセル.
*@param	obb	有向境界ボックス.
*
*@return	衝突結果を格納するResult型の値.
*/
Result TestCapsuleOBB(const Capsule& c, const OrientedBoundingBox& obb)
{
	//線分をOBBのローカル座標系に変換.
	Capsule cc = c;
	cc.seg.a -= obb.center;
	cc.seg.b -= obb.center;
	const glm::mat3 matOBB(
	glm::transpose(glm::mat3(obb.axis[0], obb.axis[1], obb.axis[2])));
	cc.seg.a = matOBB * cc.seg.a;
	cc.seg.b = matOBB * cc.seg.b;

		//衝突判定.
		Result result = TestCapsuleAABB(cc, AABB{ -obb.e, obb.e });
	if (result.isHit) {
		//衝突結果をワールド座標系に変換.
		const glm::mat3 matInvOBB(glm::inverse(matOBB));
		result.pa = matInvOBB * result.pa + obb.center;
		result.pb = matInvOBB * result.pb + obb.center;
		result.na = matInvOBB * result.na;
		result.nb = matInvOBB * result.nb;
	}
	return result;
}

/**
*球と何らかの形状の衝突判定.
*
*@param		a	判定対象のシェイプその１.
*@param		b	判定対象のシェイプその２.
*
*@return	衝突結果を格納するResult型の値.
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
*カプセルと何らかの形状の衝突判定.
*
*@param		a	判定対象のシェイプその１.
*@param		b	判定対象のシェイプその２.
*
*@return	衝突結果を格納するResult型の値.
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
*OBBと何らかの形状の衝突判定.
*
*@param	a	判定対象のシェイプその１.
*@param		b	判定対象のシェイプその２.
*
*@return	衝突結果を格納するResult型の値.
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
		//未実装.
			break;
	}
	return result;
}

/**
*シェイプ同士が衝突しているか調べる.
*
*@param		a	判定対象のシェイプその１.
*@param		b	判定対象のシェイプその２.
*
*@return	衝突結果を表すResult型の値.
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
	*シェイプ同士が衝突しているか調べる.
	*
	*@param a	判定対象のシェイプその１.
	*@param b	判定対象のシェイプその２.
	*@param pa	衝突した座標.
	*@param pb	衝突した座標.
	*
	*@retval true	衝突した.
	*@retval false	衝突しなかった.
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