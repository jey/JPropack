class TestOp implements LinearOperator {
  public int numRows() {
    return 10;
  }

  public int numCols() {
    return 20;
  }

  public void apply(boolean trans, double[] x, double[] y) {
    if(trans) {
      assert x.length == numRows();
      assert y.length == numCols();
      System.out.println("wee-transpose!");
    } else {
      assert x.length == numCols();
      assert y.length == numRows();
      System.out.println("wee!");
    }
  }
}

public class Test {
  public static void main(String[] args) {
    new JPropack().svds(new TestOp(), 2);
  }
}
