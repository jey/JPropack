interface LinearOperator {
  public int numRows();
  public int numCols();

  // y = (trans ? a.transpose() : a) * x
  public void apply(boolean trans, double[] x, double[] y);
}
