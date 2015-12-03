import breeze.linalg._

class TestOp extends LinearOperator {
  def numRows = 10
  def numCols = 20

  def apply(trans: Boolean, x: Array[Double], y: Array[Double]) = {
    if(trans) {
      assert(x.length == numRows())
      assert(y.length == numCols())
      System.out.println("wee-transpose!")
      //throw new RuntimeException("oink")
    } else {
      assert(x.length == numCols())
      assert(y.length == numRows())
      System.out.println("wee!")
    }
  }
}

object Test {
  def main(args: Array[String]) = {
    new JPropack().svds(new TestOp(), 2)
  }
}
