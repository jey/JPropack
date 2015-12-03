CFLAGS = -std=gnu99 -fopenmp -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux
#CFLAGS += -DNDEBUG

default: libjpropack.so JPropack.class LinearOperator.class Test.class
	java -enableassertions -Djava.library.path=. Test

%.class: %.java
	javac $^

JPropack.h: JPropack.class
	javah JPropack

libjpropack.so: jpropack.c
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $^ -L. -ldpropack_omp -ldlapack_util_omp -lopenblas -lgfortran

jpropack.c: JPropack.h

.PHONY: clean
clean:
	rm -f libjpropack.so JPropack.h *.o *.class
