#include <Eigen/Dense>
#include <cstdint>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cfloat>

namespace EI = ::Eigen;
typedef EI::VectorXd Vector;
typedef EI::MatrixXd Matrix;
using EI::Map;

typedef int Index;

#if defined(__x86_64__)
typedef int ftnint;
typedef int ftnlen;
#else
#error ???
#endif

typedef int (*daprod_t)(char *transa, ftnint *m, ftnint *n,
  double *x, double *y, double *dparm, ftnint *iparm,
  ftnlen transa_len);

extern "C" int dlansvd_(const char *, const char *, ftnint *, ftnint *,
   ftnint *, ftnint *, daprod_t, double *, ftnint *, double *,
  double *, double *, ftnint *, double *, double *,
  ftnint *, ftnint *, ftnint *, double *, ftnint *,
  ftnint *, double *, ftnint *, ftnlen, ftnlen);

struct PropackSVD {
  Matrix U;
  Vector S;
  Matrix Vt;
};

static int Matrix_daprod(char *transa, ftnint *m, ftnint *n, double *x,
    double *y, double *dparm, ftnint *iparm, ftnlen transa_len) {
  const Matrix &mat = *reinterpret_cast<Matrix*>(iparm);
  assert(mat.rows() == *m);
  assert(mat.cols() == *n);
  assert(transa_len == 1);
  assert(tolower(*transa) == 't' || tolower(*transa) == 'n');
  bool istrans = tolower(*transa) == 't';
  if(istrans) {
    Map<Vector>(y, *n) = mat.transpose() * Map<Vector>(x, *m);
  } else {
    Map<Vector>(y, *m) = mat * Map<Vector>(x, *n);
  }
  return 0;
}

PropackSVD lansvd(const Matrix &mat, int neig) {
  int m = mat.rows();
  int n = mat.cols();
  int kmax = std::min(m, n); /* TODO: maximum dimension of Krylov subspace */
  int ldu = m;
  double *U = (double*)malloc(sizeof(double) * ldu * (kmax + 1));
  /* zero out U to indicate we want a random starting vector */
  memset(U, 0, sizeof(double) * ldu * (kmax + 1));
  double *S = (double*)malloc(sizeof(double) * neig);
  double *bnd = (double*)malloc(sizeof(double) * neig);
  int ldv = n;
  double *V = (double*)malloc(sizeof(double) * ldv * (kmax + 1));
  double rtol = 1e-9; /* relative accuracy */
  int blocksz = 128; /* TODO: make configurable? */
  int lwork = m + n + 9*kmax + 5*kmax*kmax + 4 +
      std::max(3*kmax*kmax + 4*kmax + 4, blocksz * std::max(m, n));
  double *work = (double*)malloc(sizeof(double) * lwork);
  int liwork = 8 * kmax;
  int *iwork = (int*)malloc(sizeof(int) * liwork);
  /* Level of orthogonality to maintain among Lanczos vectors. */
  double delta = sqrt(DBL_EPSILON);
  /* During reorthogonalization, all vectors with components larger
   * than eta along the latest Lanczos vector will be purged. */
  double eta = pow(DBL_EPSILON, 0.75);
  /* Estimate of operator norm. */
  double anorm = 0.0;
  double doption[3] = { delta, eta, anorm };
  int ioption[2] = { 0, 1 };
  int info = -1;

  /** call PROPACK **/
  dlansvd_("Y", "Y", &m, &n, &neig, &kmax, Matrix_daprod, U, &ldu, S,
      bnd, V, &ldv, &rtol, work, &lwork, iwork, &liwork, doption, ioption,
      &info, NULL, (ftnint*)&mat, (ftnlen)1, (ftnlen)1);
  fprintf(stderr, "JPropack: info=%d\n", info);
  assert(info == 0); /* TODO */

  PropackSVD result;
  result.U = Map<Matrix>(U, ldu, neig);
  result.S = Map<Vector>(S, neig);
  result.Vt = Map<Matrix>(V, ldv, neig);

  free(iwork);
  free(work);
  free(V);
  free(S);
  free(U);

  return result;
}

int main() {
  Matrix mat = Matrix::Random(10, 20);
  int neig = 4;
  auto svd = mat.jacobiSvd(EI::ComputeThinU | EI::ComputeThinV);
  auto psvd = lansvd(mat, neig);
  std::cout << svd.singularValues().transpose() << std::endl;
  std::cout << psvd.S.transpose() << std::endl;
  std::cout << std::endl;
  std::cout << svd.matrixU().leftCols(neig)-psvd.U << std::endl;
}
