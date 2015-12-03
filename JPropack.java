public class JPropack {
  static {
     System.loadLibrary("jpropack");
  }

  public static native void svds(LinearOperator op, int neig,
      double[] U, double[] S, double[] V);

  // TODO: deuglify
  public static SVD svds(LinearOperator op, int neig) {
    SVD result = new SVD();
    result.U = new double[op.numRows() * neig];
    result.S = new double[neig];
    result.V = new double[op.numCols() * neig];
    svds(op, neig, result.U, result.S, result.V);
    return result;
  }
}
