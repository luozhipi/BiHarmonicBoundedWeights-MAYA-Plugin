#ifndef __ARRAY3D_H
#define __ARRAY3D_H

#include "EIGEN_inc.h"

// Minimal encapsulation for a 3D array, mostly for indexing and asserts
template<class T> class Array3D {

public:
	Array3D() {
		m_size = Vector3i::Zero();
		m_data = NULL;
	}
	Array3D(int xSize, int ySize, int zSize) {
		m_data = NULL;
		init(xSize, ySize, zSize);
	}
	~Array3D() {
		free();
	}

	void copy(const Array3D &arr) {
		m_size = arr.m_size;
		const int cLen = m_size[0] * m_size[1] * m_size[2];
		m_data = new T[cLen];
		for (int i = 0; i<cLen; i++) m_data[i] = arr.m_data[i];
	}
	Array3D(const Array3D &arr) {
		copy(arr);
	}

	Array3D& operator=(const Array3D &arr) {
		if (&arr == this) return *this; // assigning to itself
		delete[] m_data;
		copy(arr);
		return *this;
	}

	void init(int xSize, int ySize, int zSize) {
		assert(xSize >= 0 && ySize >= 0 && zSize >= 0);
		if (m_data != NULL) delete[] m_data;
		m_data = new T[xSize * ySize * zSize];
		m_size = Vector3i(xSize, ySize, zSize);
	}

	void free() {
		if (m_data != NULL) delete[] m_data;
		m_data = NULL;
		m_size = Vector3i::Zero();
	}

	bool validIndices(int x, int y, int z) const {
		return (x >= 0 && x < (int)m_size[0] && y >= 0 && y < (int)m_size[1] && z >= 0 && z < (int)m_size[2]);
	}

	T& operator()(int x, int y, int z) {
		assert(validIndices(x, y, z));
		return m_data[x + y*m_size[0] + z*m_size[0] * m_size[1]];
	}

	const T& operator()(int x, int y, int z) const {
		assert(validIndices(x, y, z));
		return m_data[x + y*m_size[0] + z*m_size[0] * m_size[1]];
	}

	int getSize(int c) const {
		return m_size[c];
	}

	void setAllTo(T cVal) {
		const int totalSize = m_size[0] * m_size[1] * m_size[2];
		for (int i = 0; i<totalSize; i++) m_data[i] = cVal;
	}

protected:

	Vector3i m_size; // x, y, z dimensions
	T * m_data;

};

#endif