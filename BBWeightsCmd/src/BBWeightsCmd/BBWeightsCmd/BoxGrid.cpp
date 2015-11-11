#include "BoxGrid.h"
#include "MOSEK_solver.h" // QP solver (Mosek library)

void BoxGrid::initVoxels(const int res, Array3D<int>& m_voxArray) {

	m_size[0] = m_size[1] = m_size[2] = res;
	m_lowerLeft.setZero();
	m_upperRight.setConstant(1.0f);
	const int & Xs = m_size[0];
	const int & Ys = m_size[1];
	const int & Zs = m_size[2];
	m_boxArray.init(Xs, Ys, Zs);
	m_nodeArray.init(Xs + 1, Ys + 1, Zs + 1);
	m_frac = (m_upperRight - m_lowerLeft).cwiseQuotient(RowVector3(Xs, Ys, Zs));

	m_boxArray.setAllTo(-1);
	nnzBoxes = 0;
	for (int x = 0; x<Xs; x++) {
		for (int y = 0; y<Ys; y++) {
			for (int z = 0; z<Zs; z++) {
				if (m_voxArray(x,y,z) != -1) {
					m_boxArray(x, y, z) = nnzBoxes++;
				}
			}
		}
	}
}
//oct-tree
void BoxGrid::initStructure() {
	const int & Xs = m_size[0];
	const int & Ys = m_size[1];
	const int & Zs = m_size[2];
	m_nodeArray.setAllTo(-1);
	nnzNodes = 0;
	for (int x = 0; x<Xs + 1; x++) {
		for (int y = 0; y<Ys + 1; y++) {
			for (int z = 0; z<Zs + 1; z++) {
				bool occupiedNeighboringBox = false;
				for (int dx = -1; dx<1; dx++) {
					for (int dy = -1; dy<1; dy++) {
						for (int dz = -1; dz<1; dz++) {
							if (m_boxArray.validIndices(x + dx, y + dy, z + dz)) {
								if (m_boxArray(x + dx, y + dy, z + dz) != -1) occupiedNeighboringBox = true;
							}
						}
					}
				}
				if (occupiedNeighboringBox) m_nodeArray(x, y, z) = nnzNodes++;
			}
		}
	}

	m_nodes.assign(nnzNodes, RowVector3(0, 0, 0));
	for (int x = 0; x<Xs + 1; x++) {
		for (int y = 0; y<Ys + 1; y++) {
			for (int z = 0; z<Zs + 1; z++) {
				const int nIdx = m_nodeArray(x, y, z);
				if (nIdx != -1) {
					m_nodes[nIdx] = m_lowerLeft + m_frac.cwiseProduct(RowVector3(x, y, z));
				}
			}
		}
	}

	m_nodeNodes.setConstant(nnzNodes, 6, -1);
	for (int x = 0; x<Xs + 1; x++) {
		for (int y = 0; y<Ys + 1; y++) {
			for (int z = 0; z<Zs + 1; z++) {
				if (m_nodeArray(x, y, z) == -1) continue;
				int idNode = m_nodeArray(x, y, z);
				int k = 0;
				for (int dw = -1; dw <= 1; dw += 2) {
					if (m_nodeArray.validIndices(x + dw, y, z)) {
						m_nodeNodes(idNode, k) = m_nodeArray(x + dw, y, z);
					}
					k++;
					if (m_nodeArray.validIndices(x, y + dw, z)) {
						m_nodeNodes(idNode, k) = m_nodeArray(x, y + dw, z);
					}
					k++;
					if (m_nodeArray.validIndices(x, y, z + dw)) {
						m_nodeNodes(idNode, k) = m_nodeArray(x, y, z + dw);
					}
					k++;
				}
			}
		}
	}

	m_boxNodes.setConstant(nnzBoxes, 8, -1);
	for (int x = 0; x<Xs; x++) {
		for (int y = 0; y<Ys; y++) {
			for (int z = 0; z<Zs; z++) {
				const int idBox = m_boxArray(x, y, z);
				if (idBox == -1) continue;
				int cnt = 0;
				for (int dx = 0; dx<2; dx++) {
					for (int dy = 0; dy<2; dy++) {
						for (int dz = 0; dz<2; dz++) {
							m_boxNodes(idBox, cnt++) = m_nodeArray(x + dx, y + dy, z + dz);
						}
					}
				}
			}
		}
	}

	m_boxBoxes.setConstant(nnzBoxes, 6, -1);
	for (int x = 0; x<Xs; x++) {
		for (int y = 0; y<Ys; y++) {
			for (int z = 0; z<Zs; z++) {
				if (m_boxArray(x, y, z) == -1) continue;
				int idBox = m_boxArray(x, y, z);
				int k = 0;
				for (int dw = -1; dw <= 1; dw += 2) {
					if (m_boxArray.validIndices(x + dw, y, z)) {
						m_boxBoxes(idBox, k) = m_boxArray(x + dw, y, z);
					}
					k++;
					if (m_boxArray.validIndices(x, y + dw, z)) {
						m_boxBoxes(idBox, k) = m_boxArray(x, y + dw, z);
					}
					k++;
					if (m_boxArray.validIndices(x, y, z + dw)) {
						m_boxBoxes(idBox, k) = m_boxArray(x, y, z + dw);
					}
					k++;
				}
			}
		}
	}
}
void BoxGrid::freeAll()
{
	m_nodeArray.free();
	m_boxArray.free();
	m_nodes.clear();
}
int BoxGrid::getBoxContainingPoint(const RowVector3 & P, RowVector3 & t) const
{
	if (P.cwiseMin(m_lowerLeft) != m_lowerLeft || P.cwiseMax(m_upperRight) != m_upperRight) {
		cout << "Error : point is outside the voxelgrid" << endl;
		//P = P.cwiseMax(m_lowerLeft).cwiseMin(m_upperRight);
		return -1;
	}
	RowVector3 floatIndices = (P - m_lowerLeft).cwiseQuotient(m_frac);
	RowVector3i intIndices((int)floor(floatIndices[0]), (int)floor(floatIndices[1]), (int)floor(floatIndices[2]));
	if (!m_boxArray.validIndices(intIndices[0], intIndices[1], intIndices[2])) {
		cout << "Error : position in voxelgrid incorrect" << endl;
		return -1;
	}
	t = floatIndices - RowVector3(intIndices[0], intIndices[1], intIndices[2]);
	return m_boxArray(intIndices[0], intIndices[1], intIndices[2]);
}

bool BoxGrid::getBoxCoordsContainingPoint(const RowVector3 & P, RowVector3i & t) const
{
	if (P.cwiseMin(m_lowerLeft) != m_lowerLeft || P.cwiseMax(m_upperRight) != m_upperRight) {
		cout << "Error : point is outside the voxelgrid" << endl;
		//P = P.cwiseMax(m_lowerLeft).cwiseMin(m_upperRight);
		return false;
	}
	RowVector3 floatIndices = (P - m_lowerLeft).cwiseQuotient(m_frac);
	t = RowVector3i((int)floor(floatIndices[0]), (int)floor(floatIndices[1]), (int)floor(floatIndices[2]));

	if (!m_boxArray.validIndices(t[0], t[1], t[2])) {
		cout << "Error : position in voxelgrid incorrect" << endl;
		return false;
	}
	return true;
}

int BoxGrid::getNodeClosestToPoint(const RowVector3 & P) const
{
	if (P.cwiseMin(m_lowerLeft) != m_lowerLeft || P.cwiseMax(m_upperRight) != m_upperRight) {
		cout << "Error : point is outside the voxelgrid" << endl;
		//P = P.cwiseMax(m_lowerLeft).cwiseMin(m_upperRight);
		return -1;
	}
	RowVector3 floatIndices = (P - m_lowerLeft).cwiseQuotient(m_frac);
	RowVector3i intIndices((int)floor(floatIndices[0]), (int)floor(floatIndices[1]), (int)floor(floatIndices[2]));
	if (!m_boxArray.validIndices(intIndices[0], intIndices[1], intIndices[2])) {
		cout << "Error : position in voxelgrid incorrect" << endl;
		return -1;
	}
	RowVector3 t = floatIndices - RowVector3(intIndices[0], intIndices[1], intIndices[2]);
	int dx = (t[0] <= 0.5) ? 0 : 1;
	int dy = (t[1] <= 0.5) ? 0 : 1;
	int dz = (t[2] <= 0.5) ? 0 : 1;
	return m_nodeArray(intIndices[0] + dx, intIndices[1] + dy, intIndices[2] + dz);
}

const RowVector3 & BoxGrid::getNodePose(int idNode) const {
	return m_nodes[idNode];
}

void BoxGrid::computeBoxPositions() {
	m_boxPositions.setZero(getNumBoxes(), 3);
#pragma omp parallel for
	for (int i = 0; i < getNumBoxes(); i++) {
		for (int k = 0; k < 8; ++k) {
			m_boxPositions.row(i) += getNodePose(m_boxNodes(i, k));
		}
	}
	m_boxPositions *= 0.125;
}
void BoxGrid::laplacianMEL(vector<SparseMatrixTriplet> &MEL) const
{
	MEL.clear();
	vector<int> valences;
	valences.assign(getNumNodes(), 0);
	for (int v1 = 0; v1<getNumNodes(); ++v1) {
		for (int k = 0; k < 6; ++k) {
			int v2 = m_nodeNodes(v1, k);
			if (v2 != -1) {
				MEL.push_back(SparseMatrixTriplet(v1, v2, -1.0f));
				valences[v1]++;
			}
		}
	}
	for (int v = 0; v<getNumNodes(); v++) {
		MEL.push_back(SparseMatrixTriplet(v, v, valences[v]));
	}
}
//Laplace¨CBeltrami operator, when applied to a function, is the trace of the function's Hessian:
//Laplacian energy minimization Dirichlet energy functional stationary:
//biharmonic second order of harmonic, fourth-order partial differential equation
void BoxGrid::computeBBW(map<string, RowVector3> B)
{
	int N = getNumNodes();
	int M = B.size();
	// each voxel node will have weights
	m_weights.clear();
	m_weights.resize(N);
	// compute the Laplacian matrix
	SparseMatrix L, L2;
	vector<SparseMatrixTriplet> L_MEL;
	laplacianMEL(L_MEL);//second-order
	L.resize(N, N);
	L.setFromTriplets(L_MEL.begin(), L_MEL.end());
	L2 = L*L;//fourth-order
	// compute the constraint matrix (each row corresponds to one handle)
	SparseMatrix A(M, N);
	vector<SparseMatrixTriplet> A_MEL;
	A_MEL.reserve(M);
	int i = 0;
	for (map<string, RowVector3>::iterator it = B.begin(); it != B.end(); it++)
	{
		A_MEL.push_back(SparseMatrixTriplet(i, getNodeClosestToPoint(it->second), 1.0));
		i++;
	}
	A.setFromTriplets(A_MEL.begin(), A_MEL.end());
	VectorX x;
	VectorX b;
	MOSEKinterface mi0;
	MatrixXX zeros0(L2.rows(), 1); zeros0.setZero();
	for (int j = 0; j < M; ++j) { // for each handle we compute the weight
		b.setZero(M, 1);
		b[j] = 1.0f;
		x.setZero(N, 1);			// 1: bounded else just biharmonic
		mi0.solveQP_BBW_type(x, L2, zeros0, A, b, 1, MOSEKinterface::PRINT_NOTHING); // call Mosek QP solver
		for (int i = 0; i < N; i++) { // we push the j-th weight for each node
			m_weights[i].pushWeight((ScalarType)x[i]);
		}
	}
	// for each node we now have all the weight so we can normalize them
	for (int i = 0; i < N; i++) {
		m_weights[i].normalizeWeights();
	}
}
//this is implemented by Zhiping 11/12/2014
void BoxGrid::computeBoneBBW(map<string, RowVector3> B, map<string, string> boneWise)
{
	vector<int> bones;
	vector<RowVector3> boneLocs;
	int number = 0;
	map<string, int> b_index;
	for (map<string, RowVector3>::iterator it = B.begin(); it != B.end(); it++)
	{
		b_index[it->first] = number;
		number++;
	}
	for (map<string, string>::iterator it = boneWise.begin(); it != boneWise.end(); it++)
	{
		bones.push_back(b_index[it->first]);
		RowVector3 pLoc = B[it->first];
		RowVector3 cLoc = B[it->second];
		boneLocs.push_back((cLoc + pLoc) / 2.0);
	}
	int N = getNumNodes();
	int M = bones.size();
	// each voxel node will have weights
	m_weights.clear();
	m_weights.resize(N);
	// compute the Laplacian matrix
	SparseMatrix L, L2;
	vector<SparseMatrixTriplet> L_MEL;
	laplacianMEL(L_MEL);//second-order
	L.resize(N, N);
	L.setFromTriplets(L_MEL.begin(), L_MEL.end());
	L2 = L*L;//fourth-order
	// compute the constraint matrix (each row corresponds to one handle)
	SparseMatrix A(M, N);
	vector<SparseMatrixTriplet> A_MEL;
	A_MEL.reserve(M);
	for (int i = 0; i < M; ++i) {
		RowVector3 boneLoc = boneLocs[i];
		int idNode = getNodeClosestToPoint(boneLoc);
		A_MEL.push_back(SparseMatrixTriplet(i, idNode, 1.0));
	}
	A.setFromTriplets(A_MEL.begin(), A_MEL.end());
	VectorX x;
	VectorX b;
	MOSEKinterface mi0;
	MatrixXX zeros0(L2.rows(), 1); zeros0.setZero();
	for (int j = 0; j < M; ++j) { // for each handle we compute the weight
		b.setZero(M, 1);
		b[j] = 1.0f;
		x.setZero(N, 1);			// 1: bounded else just biharmonic
		mi0.solveQP_BBW_type(x, L2, zeros0, A, b, 1, MOSEKinterface::PRINT_NOTHING); // call Mosek QP solver
		for (int i = 0; i < N; i++) { // we push the j-th weight for each node
			m_weights[i].pushWeight((ScalarType)x[i]);
		}
	}
	// for each node we now have all the weight so we can normalize them
	for (int i = 0; i < N; i++) {
		m_weights[i].normalizeWeights();
	}
	vector<Weights> tmp_weights = m_weights;
	tmp_weights.clear(); tmp_weights.resize(N);
	for (int j = 0; j<B.size(); j++)
	{
		for (int i = 0; i < N; i++)
		{
			tmp_weights[i].pushWeight(0.0);
		}
	}
	for (int j = 0; j<M; j++)
	{
		for (int i = 0; i < N; i++)
		{
			tmp_weights[i].setCoord(bones[j], m_weights[i].getCoord(j));
		}
	}
	m_weights = tmp_weights; tmp_weights.clear();
	for (int i = 0; i < N; i++) {
		m_weights[i].setSumCoords();
	}
}
void BoxGrid::getInterpolatedBBW(const RowVector3 & P, Weights & deformInfo, const int nbWeights) const {
	RowVector3 t;
	int idBox = getBoxContainingPoint(P, t);
	if (idBox == -1) {
		cout << "Error cannot create BBW for this mesh vertex" << endl;
		return;
	}
	for (int j = 0; j < nbWeights; ++j) {
		float wj = 0.0f;
		for (int i = 0; i < 8; ++i) {
			const int idNode = m_boxNodes(idBox, i);
			float alpha = 1.0f;
			alpha *= (((i / 4) % 2 == 0) ? (1.0f - t[0]) : t[0]);
			alpha *= (((i / 2) % 2 == 0) ? (1.0f - t[1]) : t[1]);
			alpha *= ((i % 2 == 0) ? (1.0f - t[2]) : t[2]);
			wj += alpha * m_weights[idNode].getCoord(j);
		}
		deformInfo.pushWeight(wj);
	}
	deformInfo.normalizeWeights();
}