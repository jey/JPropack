public class JPropack {
  static {
     System.loadLibrary("jpropack");
  }

  public native void svds(LinearOperator op, int k);
}
