// Interface to MOSEK

#ifndef __MOSEKinterface_H__
#define __MOSEKinterface_H__

#include "mosek.h"

#include "EIGEN_inc.h"

class MOSEKinterface
{
public:
	MOSEKinterface();
	virtual ~MOSEKinterface();

	enum LOGtype { PRINT_LOG, PRINT_NOTHING };

	bool solveQP_BBW_type(VectorX &X, const SparseMatrix &Q, const MatrixXX &C, const SparseMatrix &A, const VectorX &b, int variableBounds, LOGtype logtype);


private:
	MSKenv_t    m_env;

};

#endif // __MOSEKinterface_H__
