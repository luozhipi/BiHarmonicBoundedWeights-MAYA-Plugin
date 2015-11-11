#ifndef __WEIGHTS_H
#define __WEIGHTS_H

#include "STL_inc.h"

// simple class to store the weights
class Weights {

private:
	vector<ScalarType> m_coords;
	ScalarType m_sumCoords;

public:
	Weights() {
		m_sumCoords = 0;
		m_coords.clear();
	}
	~Weights() {}

	void pushWeight(ScalarType w) {
		m_coords.push_back(w);
		m_sumCoords += w;
	}
	vector<ScalarType> getCoords() const{ return m_coords; }
	ScalarType getCoord(int b_id) const { return m_coords[b_id]; }
	void setCoord(int b_id, ScalarType w){ m_coords[b_id] = w; }
	void setSumCoords(){ m_sumCoords = 1.0; }
	void normalizeWeights() {
		for (unsigned int i = 0; i < m_coords.size(); i++) {
			m_coords[i] /= m_sumCoords;
		}
		m_sumCoords = 1.0;
	}

};

#endif