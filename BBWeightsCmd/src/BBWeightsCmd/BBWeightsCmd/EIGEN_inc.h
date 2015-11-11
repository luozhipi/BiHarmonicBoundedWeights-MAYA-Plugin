#ifndef _EIGEN_h
#define _EIGEN_h

//#define EIGEN_USE_MKL_ALL

#define EIGEN_DONT_VECTORIZE
#define EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT

#include <Eigen/Core>
#include <Eigen/Sparse>


#define USE_DOUBLE

typedef int IndexType;

#ifdef USE_DOUBLE
typedef double ScalarType;
#define TW_TYPE_SCALARTYPE TW_TYPE_DOUBLE
#else
typedef float ScalarType;
#define TW_TYPE_SCALARTYPE TW_TYPE_FLOAT
#endif

typedef Eigen::Matrix<ScalarType, Eigen::Dynamic, 3, Eigen::RowMajor> PointMatrixType;
typedef Eigen::Matrix<IndexType, Eigen::Dynamic, 3, Eigen::RowMajor> FaceMatrixType;
typedef Eigen::Matrix<IndexType, Eigen::Dynamic, 4, Eigen::RowMajor> QuadMatrixType;
typedef Eigen::Matrix<ScalarType, Eigen::Dynamic, 2, Eigen::RowMajor> UVMatrixType;

typedef Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic> MatrixXX;
typedef Eigen::Matrix<ScalarType, Eigen::Dynamic, 2>              MatrixX2;
typedef Eigen::Matrix<ScalarType, Eigen::Dynamic, 3>              MatrixX3;
typedef Eigen::Matrix<ScalarType, 2, 2>              Matrix22;
typedef Eigen::Matrix<ScalarType, 3, 3>              Matrix33;
typedef Eigen::Matrix<ScalarType, 4, 4>              Matrix44;

typedef Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> VectorX;
typedef Eigen::Matrix<ScalarType, 2, 1> Vector2;
typedef Eigen::Matrix<ScalarType, 3, 1> Vector3;
typedef Eigen::Matrix<ScalarType, 4, 1> Vector4;

typedef Eigen::Matrix<ScalarType, 1, Eigen::Dynamic> RowVectorX;
typedef Eigen::Matrix<ScalarType, 1, 2>              RowVector2;
typedef Eigen::Matrix<ScalarType, 1, 3>              RowVector3;
typedef Eigen::Matrix<ScalarType, 1, 4>              RowVector4;

typedef Eigen::Matrix<IndexType, Eigen::Dynamic, Eigen::Dynamic> MatrixXXi;
typedef Eigen::Matrix<IndexType, Eigen::Dynamic, 3>              MatrixX3i;
typedef Eigen::Matrix<IndexType, Eigen::Dynamic, 2>              MatrixX2i;
typedef Eigen::Matrix<IndexType, Eigen::Dynamic, 12>             MatrixX12i;
typedef Eigen::Matrix<IndexType, Eigen::Dynamic, 8>              MatrixX8i;
typedef Eigen::Matrix<ScalarType, Eigen::Dynamic, 8>             MatrixX8f;
typedef Eigen::Matrix<ScalarType, Eigen::Dynamic, Eigen::Dynamic>   MatrixXXf;
typedef Eigen::Matrix<IndexType, Eigen::Dynamic, 6>              MatrixX6i;
typedef Eigen::Matrix<IndexType, Eigen::Dynamic, 4>              MatrixX4i;

typedef Eigen::Matrix<IndexType, Eigen::Dynamic, 1> VectorXi;
typedef Eigen::Matrix<IndexType, 3, 1> Vector3i;
typedef Eigen::Matrix<IndexType, 1, 3> RowVector3i;

typedef Eigen::AngleAxis<ScalarType> AngleAxis;
typedef Eigen::Quaternion<ScalarType> Quaternion;

typedef Eigen::SparseMatrix<ScalarType> SparseMatrix;
typedef Eigen::Triplet<ScalarType, IndexType> SparseMatrixTriplet;

typedef Eigen::Matrix<ScalarType, Eigen::Dynamic, 3, Eigen::RowMajor> RowMatrixX3;
typedef Eigen::Matrix<ScalarType, 8, 3, Eigen::RowMajor> RowMatrix83;
typedef Eigen::Matrix<ScalarType, 8, 4, Eigen::RowMajor> RowMatrix84;
typedef Eigen::Matrix<ScalarType, Eigen::Dynamic, 4, Eigen::RowMajor> RowMatrixX4;
typedef Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> RowMatrixX1;

#endif

