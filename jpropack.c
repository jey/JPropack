/* NOTE: assumes f2c calling convention for now */
#include <jni.h>
#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "JPropack.h"

/* include last because it defines weird macros */
#include <f2c.h>

#ifdef __GNUC__
#define UNLIKELY(x) __builtin_expect((x), 0)
#else
#define UNLIKELY(x) (x)
#endif

#ifdef NDEBUG
#define ENSURE(x) \
  do if(!UNLIKELY(x)) { \
    fprintf(stderr, "%s:%d: FATAL: invariant failed: " #x "\n", \
        __FILE__, __LINE__); \
    abort(); \
  } while(0)
#else
#define ENSURE(x) assert(x)
#endif

typedef int (*daprod_t)(char *transa, integer *m, integer *n, 
  doublereal *x, doublereal *y, doublereal *dparm, integer *iparm, 
  ftnlen transa_len);

extern int dlansvd_(char *, char *, integer *, integer *,
   integer *, integer *, daprod_t, doublereal *, integer *, doublereal *,
  doublereal *, doublereal *, integer *, doublereal *, doublereal *,
  integer *, integer *, integer *, doublereal *, integer *, 
  integer *, doublereal *, integer *, ftnlen, ftnlen);

typedef struct {
  JNIEnv *env;
  jobject opObj;
  jobject opClass;
  jmethodID applyFcn;
} LinearOperator;

static int LinearOperator_daprod(char *transa, integer *m, integer *n, 
    doublereal *x, doublereal *y, doublereal *dparm, integer *iparm, 
    ftnlen transa_len) {
  LinearOperator *ctx = (LinearOperator*) iparm;
  JNIEnv *env = ctx->env;
  assert(tolower(*transa) == 't' || tolower(*transa) == 'n');
  jboolean istrans = tolower(*transa) == 't';
  int xlen = istrans ? *m : *n;
  int ylen = istrans ? *n : *m;
  /* TODO: reuse arrays, error checking */
  jdoubleArray jx = (*env)->NewDoubleArray(env, xlen);
  jdoubleArray jy = (*env)->NewDoubleArray(env, ylen);
  ENSURE(jx && jy);
  (*env)->SetDoubleArrayRegion(env, jx, 0, xlen, x);
  (*env)->CallVoidMethod(env, ctx->opObj, ctx->applyFcn, istrans, jx, jy);
  ENSURE(!(*env)->ExceptionOccurred(env)); /* TODO */
  assert((*env)->GetArrayLength(env, jy) == ylen);
  double *jyarr = (jdouble*)(*env)->GetPrimitiveArrayCritical(env, jy, 0);
  memcpy(y, jyarr, sizeof(double) * ylen);
  (*env)->ReleasePrimitiveArrayCritical(env, jy, jyarr, 0);
  return 0;
}

JNIEXPORT void JNICALL Java_JPropack_svds(JNIEnv *env, jobject thisObj,
    jobject opObj, jint neig) {
  assert(sizeof(jint) == sizeof(int));
  assert(sizeof(integer) == sizeof(int));
  assert(sizeof(doublereal) == sizeof(double));
  jclass opClass = (*env)->GetObjectClass(env, opObj);
  jmethodID numRowsFcn = (*env)->GetMethodID(env, opClass, "numRows", "()I");
  jmethodID numColsFcn = (*env)->GetMethodID(env, opClass, "numCols", "()I");
  jmethodID applyFcn = (*env)->GetMethodID(env, opClass, "apply", "(Z[D[D)V");
  ENSURE(numRowsFcn && numColsFcn);
  int m = (*env)->CallIntMethod(env, opObj, numRowsFcn);
  int n = (*env)->CallIntMethod(env, opObj, numColsFcn);
  ENSURE(m > 0 && n > 0);
  LinearOperator ctx = { env, opObj, opClass, applyFcn };
  int kmax = neig; /* maximum dimension of Krylov subspace */
  int ldu = m;
  double *U = malloc(sizeof(double) * ldu * (kmax + 1));
  /* zero out U to indicate we want a random starting vector */
  memset(U, 0, sizeof(double) * ldu * (kmax + 1));
  double *S = malloc(sizeof(double) * neig);
  double *bnd = malloc(sizeof(double) * neig);
  int ldv = n;
  double *V = malloc(sizeof(double) * ldv * (kmax + 1));
  double rtol = 1e-9; /* relative accuracy */
  int blocksz = 128; /* TODO: make configurable? */
  int lwork = m + n + 9*kmax + 5*kmax*kmax + 4 +
      max(3*kmax*kmax + 4*kmax + 4, blocksz * max(m, n));
  double *work = malloc(sizeof(double) * lwork);
  int liwork = 8 * kmax;
  int *iwork = malloc(sizeof(int) * liwork);
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
  dlansvd_("y", "y", &m, &n, &neig, &kmax, &LinearOperator_daprod, U, &ldu, S,
      bnd, V, &ldv, &rtol, work, &lwork, iwork, &liwork, doption, ioption,
      &info, NULL, (void*)&ctx, (ftnlen)1, (ftnlen)1);
  printf("survived %d\n", info);
}
