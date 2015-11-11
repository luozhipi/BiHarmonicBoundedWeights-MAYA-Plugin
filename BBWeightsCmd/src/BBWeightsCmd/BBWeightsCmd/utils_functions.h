#include "EIGEN_inc.h"

inline void convertSparseMatrixToBuffer(const SparseMatrix & A, bool symmetric, int & AnumEl, int * & Asubi, int * & Asubj, double * & Aval) {
	AnumEl = 0;
	for (int k = 0; k<A.outerSize(); ++k) {
		for (SparseMatrix::InnerIterator it(A, k); it; ++it) {
			if (!symmetric || it.row() >= it.col()) AnumEl++;
		}
	}

	Asubi = new int[AnumEl];
	Asubj = new int[AnumEl];
	Aval = new double[AnumEl];

	int id = 0;
	for (int k = 0; k<A.outerSize(); ++k) {
		for (SparseMatrix::InnerIterator it(A, k); it; ++it) {
			if (!symmetric || it.row() >= it.col()) {
				Asubi[id] = it.row();
				Asubj[id] = it.col();
				Aval[id] = (double)it.value();
				id++;
			}
		}
	}
}