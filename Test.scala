import breeze.linalg._

class TestOp(mat: DenseMatrix[Double]) extends LinearOperator {
  def numRows = 10
  def numCols = 20

  def apply(trans: Boolean, x0: Array[Double], y0: Array[Double]) = {
    if(trans) {
      assert(x0.length == numRows())
      assert(y0.length == numCols())
    } else {
      assert(x0.length == numCols())
      assert(y0.length == numRows())
    }
    val x = new DenseVector(x0)
    val y = (if(trans) mat.t else mat) * x
    Array.copy(y.data, 0, y0, 0, y0.length)
  }
}

object Test {
  def main(args: Array[String]) = {
    val mat = DenseMatrix.rand(10, 20)
    val neig = 3
    new JPropack().svds(new TestOp(mat), neig)
    val refsvd = svd(mat)
    System.out.println(refsvd.S(0 until neig))
  }
}
