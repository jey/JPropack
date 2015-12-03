### CPU specific flags for gcc/gfortran.
CPUOPT = -march=native

# Optimization flags
OPT = -O3 -funroll-all-loops -ffast-math $(CPUOPT)
OPT_ACCURATE = -O3 -funroll-all-loops $(CPUOPT)
NO_OPT = -O0

# Compiler flags
FFLAGS = -g -fPIC -fopenmp -fno-second-underscore  -W $(OPT)
FFLAGS_NOOPT = -g -ff2c -fPIC -fopenmp -fno-second-underscore -W $(NOOPT) 
FFLAGS_ACCURATE = -g -ff2c -fPIC -fopenmp -fno-second-underscore -W $(OPT_ACCURATE)
CFLAGS = -g -fPIC -fopenmp  -W  $(OPT) 
LINKFLAGS = -g -fPIC -fopenmp  -W  $(OPT) 

# Which compiler and linker to use
CC = gcc
F77 = gfortran 
LINKER = gfortran 
MAKELIB = ar rc

# Linker paths and flags
LIBPATH = 
BLAS = -lopenblas
RANLIB = ranlib
MGS = mgs.pentium

#  Platform postfix to use in naming executables and library files
PLAT = omp
