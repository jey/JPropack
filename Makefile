CFLAGS = -std=gnu99 -fopenmp -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux
#CFLAGS += -DNDEBUG

default: jpropack.jar
	# this is silly, but easier than setting up mvn or sbt to pull in breeze
	spark-submit --verbose --driver-library-path . --class Test jpropack.jar

jpropack.jar: libjpropack.so JPropack.class LinearOperator.class Test.class
	jar cvf $@ *.class

%.class: %.java
	javac $^

%.class: %.scala
	scalac $^

JPropack.h: JPropack.class
	javah JPropack

libjpropack.so: jpropack.c
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^ -L. -ldpropack_f2c_omp -ldlapack_util_f2c_omp -lopenblas -lgfortran

jpropack.c: JPropack.h
Test.scala: JPropack.class LinearOperator.class

.PHONY: clean
clean:
	rm -f libjpropack.so JPropack.h *.o *.class jpropack.jar
