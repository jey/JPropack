import breeze.linalg._
import breeze.stats.distributions.Gaussian

class TestOp(mat: DenseMatrix[Double]) extends LinearOperator {
  override def numRows = mat.rows
  override def numCols = mat.cols

  override def apply(trans: Boolean, x0: Array[Double], y0: Array[Double]) = {
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
    val mat = DenseMatrix.rand(20, 10, Gaussian(0, 1))
    val neig = 4
    val test = JPropack.svds(new TestOp(mat), neig)
    val testU = new DenseMatrix(mat.rows, neig, test.U)
    val testS = new DenseVector(test.S)
    val testVt = new DenseMatrix(mat.cols, neig, test.V)
    val ref = breeze.linalg.svd(mat)
    println("S ref:")
    println(ref.S)
    println("\nS error:")
    println((ref.S(0 until neig) - testS))
    println("\nU error:")
    println((ref.U(::, 0 until neig) - testU))
    println("\nV error:")
    println((ref.Vt(::, 0 until neig) - testVt))
  }
}
