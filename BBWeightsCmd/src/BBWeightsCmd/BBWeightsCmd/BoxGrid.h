#ifndef __BOXGRID_H
#define __BOXGRID_H

#include "STL_inc.h"
#include "Array3D.h"
#include "Weights.h"
#include "EIGEN_inc.h"

// Basic data structures for a 3D grid of regular boxes (not necessarily equilateral -- though some methods silently assume square boxes)
// Some boxes can be empty, so we distinguish all elements (i.e. full 3D array) and non-empty ones (carving a subset of the 3D array)
class BoxGrid {

public:
	BoxGrid(){}
	~BoxGrid() {
		freeAll();
	}

	void initVoxels(const int res, Array3D<int>& m_voxArray);
	void initStructure();

	int getNumNodes() const { return nnzNodes; }
	int getNumBoxes() const { return nnzBoxes; }

	void getInterpolatedBBW(const RowVector3 & P, Weights & deformInfo, const int nbWeights) const;
	void computeBBW(map<string, RowVector3> B);
	void computeBoneBBW(map<string, RowVector3> B, map<string, string> boneWise);
	void laplacianMEL(vector<SparseMatrixTriplet> &MEL) const;
	float getWeight(int idHandle, int idNode) const;

	int getBoxContainingPoint(const RowVector3 & P, RowVector3 & t) const;
	bool getBoxCoordsContainingPoint(const RowVector3 & P, RowVector3i & t) const;
	int getNodeClosestToPoint(const RowVector3 & P) const;

	void computeBoxPositions();
	RowMatrixX3::ConstRowXpr getBoxPosition(int idBox) const { return m_boxPositions.row(idBox); }
	const RowVector3 & getNodePose(int idNode) const;


	int getBoxBoxes(int idBox, int idNeighbor) const { return m_boxBoxes(idBox, idNeighbor); }
	int getNodeNodes(int idNode, int idNeighbor) const { return m_nodeNodes(idNode, idNeighbor); }
	int getBoxNodes(int idBox, int idNeighbor) const { return m_boxNodes(idBox, idNeighbor); }

	void exportBBW(string filename);

protected:
	Vector3i m_size; // number of boxes in x,y,z dimensions
	RowVector3 m_lowerLeft, m_upperRight; // placement in 3D space
	RowVector3 m_frac;
	Array3D<int> m_nodeArray; // nodes 
	Array3D<int> m_boxArray; // boxes
	int nnzBoxes, nnzNodes, nnzEdges[3];
	void freeAll();
	vector<RowVector3> m_nodes;
	RowMatrixX3 m_boxPositions; // positions of boxes (isobarycenter)
	MatrixX8i m_boxNodes; // numBoxes x 8 int matrix of node indices (incident to a given box)
	MatrixX6i m_nodeNodes; // numNodes x 6 int matrix of node indices (incident to a given node)
	MatrixX6i m_boxBoxes;
	vector<Weights> m_weights;
};

#endif
