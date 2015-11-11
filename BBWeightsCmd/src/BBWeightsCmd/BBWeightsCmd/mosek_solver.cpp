#include "mosek_solver.h"

#include "utils_functions.h"

static void MSKAPI printstr(void *handle,
	MSKCONST char str[])
{
	printf("%s", str);
}

MOSEKinterface::MOSEKinterface()
{
	MSKrescodee   r;
	r = MSK_makeenv(&m_env, NULL);
	r = MSK_initenv(m_env);
	r = MSK_linkfunctoenvstream(m_env, MSK_STREAM_LOG, NULL, printstr);
}

MOSEKinterface::~MOSEKinterface()
{
	MSK_deleteenv(&m_env);
}

void MSK_GUARDED(MSKrescodee r)
{
	if (r != MSK_RES_OK)
	{
		/* In case of an error print error code and description. */
		char symname[MSK_MAX_STR_LEN];
		char desc[MSK_MAX_STR_LEN];

		printf("An error occurred while optimizing.\n");
		MSK_getcodedesc(r, symname, desc);
		printf("Error %s - '%s'\n", symname, desc);
		assert(r == MSK_RES_OK);
	}
}

/*void alecsMosekParameters(MSKtask_t &task)
{
printf("WARNING: setting Alec's MOSEK parameters\n");
// set tolerance
//MSK_GUARDED(
//  MSK_putdouparam(task,MSK_DPAR_INTPNT_TOL_DFEAS,1e-8));
//MSK_GUARDED(
//  MSK_putdouparam(task,MSK_DPAR_INTPNT_TOL_DSAFE,1.0));
// 1.0 means optimizer is very leniant about declaring model infeasible
//MSK_GUARDED(
//  MSK_putdouparam(task,MSK_DPAR_INTPNT_TOL_INFEAS,1e-8));
//MSK_GUARDED(
//  MSK_putdouparam(task,MSK_DPAR_INTPNT_TOL_PATH,1e-8));
//MSK_GUARDED(
//  MSK_putdouparam(task,MSK_DPAR_INTPNT_TOL_PFEAS,1e-8));

// Hard to say if this is doing anything, probably nothing dramatic
MSK_GUARDED(MSK_putdouparam(task,MSK_DPAR_INTPNT_TOL_PSAFE,1e2));

// >1e0 NONSOLUTION
// 1e-1 artifacts in deformation
// 1e-3 artifacts in isolines
// 1e-4 seems safe
// 1e-8 MOSEK DEFAULT SOLUTION
MSK_GUARDED(MSK_putdouparam(task,MSK_DPAR_INTPNT_TOL_REL_GAP,1e-8));
//MSK_GUARDED(MSK_putdouparam(task,MSK_DPAR_INTPNT_TOL_REL_STEP,0.9999));

// Turn off presolving
//MSK_GUARDED(
//  MSK_putintparam(task,MSK_IPAR_PRESOLVE_USE,MSK_PRESOLVE_MODE_OFF));

// Force particular matrix reordering method
// MSK_ORDER_METHOD_NONE cuts time in half roughly, since half the time is
//   usually spent reordering the matrix
// !! WARNING Setting this parameter to anything but MSK_ORDER_METHOD_FREE
//   seems to have the effect of setting it to MSK_ORDER_METHOD_NONE
//   *Or maybe Mosek is spending a bunch of time analyzing the matrix to
//   choose the right ordering method when really any of them are
//   instantaneous

MSK_GUARDED(
MSK_putintparam(task,MSK_IPAR_INTPNT_ORDER_METHOD,MSK_ORDER_METHOD_NONE));


// Turn off convexity check
MSK_GUARDED(
MSK_putintparam(task,MSK_IPAR_CHECK_CONVEXITY,MSK_CHECK_CONVEXITY_NONE));

//// Force using multiple threads, not sure if MOSEK is properly destorying
////extra threads...
//MSK_GUARDED(MSK_putintparam(task,MSK_IPAR_INTPNT_NUM_THREADS,2));

// Force turn off data check
MSK_GUARDED(MSK_putintparam(task,MSK_IPAR_DATA_CHECK,MSK_OFF));
}*/


bool MOSEKinterface::solveQP_BBW_type(VectorX &X, const SparseMatrix &Q, const MatrixXX &C, const SparseMatrix &A, const VectorX &b, int variableBounds, LOGtype logtype)
{
	const int NUMCON = A.rows();
	const int NUMVAR = A.cols();
	const int NUMRHS = C.cols();
	assert(Q.rows() == NUMVAR && Q.cols() == NUMVAR);
	assert(b.rows() == NUMCON);
	assert(C.rows() == NUMVAR);
	assert(b.cols() == 1);

	MSKtask_t     task;
	MSKrescodee   r;
	r = MSK_maketask(m_env, NUMCON, NUMVAR, &task);

	r = MSK_putintparam(task, MSK_IPAR_NUM_THREADS, 4);
	r = MSK_putintparam(task, MSK_IPAR_CHECK_CONVEXITY, MSK_CHECK_CONVEXITY_SIMPLE);

	if (logtype == PRINT_LOG) r = MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);
	else if (logtype == PRINT_NOTHING) r = MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, NULL);
	r = MSK_putmaxnumvar(task, NUMVAR);
	r = MSK_putmaxnumcon(task, NUMCON);
	r = MSK_putmaxnumanz(task, A.nonZeros());

	r = MSK_appendcons(task, NUMCON);
	r = MSK_appendvars(task, NUMVAR);

	int QnumEl = 0, *Qsubi = NULL, *Qsubj = NULL; double *Qval = NULL;
	convertSparseMatrixToBuffer(Q, true, QnumEl, Qsubi, Qsubj, Qval);
	for (int i = 0; i<QnumEl; i++) Qval[i] *= 2.0;
	r = MSK_putqobj(task, QnumEl, Qsubi, Qsubj, Qval);
	delete[] Qsubi; delete[] Qsubj; delete[] Qval;

	for (int k = 0; k<NUMVAR; k++)
	{
		if (variableBounds == 1) r = MSK_putbound(task, MSK_ACC_VAR, k, MSK_BK_RA, 0.0, 1.0); // [0, 1] box constraints:
		else r = MSK_putbound(task, MSK_ACC_VAR, k, MSK_BK_FR, -MSK_INFINITY, +MSK_INFINITY); // need to explicitly tell MOSEK that we want NO bounds on *variables*:
	}

	int AnumEl = 0, *Asubi = NULL, *Asubj = NULL; double *Aval = NULL;
	convertSparseMatrixToBuffer(A, false, AnumEl, Asubi, Asubj, Aval);
	r = MSK_putaijlist(task, AnumEl, Asubi, Asubj, Aval);
	delete[] Asubi; delete[] Asubj; delete[] Aval;

	for (int k = 0; k<NUMCON; k++)
	{
		r = MSK_putbound(task, MSK_ACC_CON, k, MSK_BK_FX, b.coeffRef(k, 0), b.coeffRef(k, 0));
	}

	//alecsMosekParameters(task);

	X.setZero(NUMVAR, NUMRHS);
	MatrixXX sol(NUMVAR, 1);
	for (int i = 0; i<NUMRHS; i++)
	{
		for (int j = 0; j<NUMVAR; j++) r = MSK_putcj(task, j, C(j, i));

		//// tst
		//int numThreads = -1;
		//MSK_getintpntnumthreads(task, &numThreads);
		//r = MSK_analyzeproblem(task, MSK_STREAM_LOG);
		//// eof tst

		MSKrescodee trmcode;
		/* Run optimizer */
		r = MSK_optimizetrm(task, &trmcode);

		/* Print a summary containing information about the solution for debugging purposes*/
		if (logtype == PRINT_LOG) MSK_solutionsummary(task, MSK_STREAM_LOG);

		MSKsolstae solsta;

		MSK_getsolsta(task, MSK_SOL_ITR, &solsta);
		if (solsta != MSK_SOL_STA_OPTIMAL && solsta != MSK_SOL_STA_NEAR_OPTIMAL)
		{
			printf("solveQP failed\n");
			return false;
		}

		double * solValue = new double[NUMVAR];
		MSK_getsolutionslice(task, MSK_SOL_ITR, MSK_SOL_ITEM_XX, 0, NUMVAR, solValue);
		for (int j = 0; j<NUMVAR; j++) X.coeffRef(j, i) = solValue[j];

		delete[] solValue;
	}
	return true;
}