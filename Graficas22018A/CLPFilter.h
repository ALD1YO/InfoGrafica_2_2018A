#pragma once
class CLPFilter
{
public:
	float m_y;	// y (i+1) = y(i)+(x(i)- y (i)) *k
	float m_k;
public:
	CLPFilter(float y0, float k) { m_y = y0; m_k = k; }
	void Time(float i) {m_y = m_y + (i-m_y) *m_k; }
	~CLPFilter();
};

