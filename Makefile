# -------------------------------------------------
# Variables setting
# -------------------------------------------------
SYSTEM = $(shell uname)
OPT = release
#OPT = debug
CXX = g++
TARGET = autoDetect

INCDIR=	\
		-I./inc \
		-I/usr/local/include \
		-I/usr/local/include/opencv \
		-I/usr/local/include/opencv2

SRCDIR=	\
		src

LIBDIR=	\
		-L/usr/local/cuda/lib64 \
		-L/usr/local/jpegdev/lib \
		-lopencv_core -lopencv_highgui -lopencv_ml -lopencv_calib3d -lopencv_imgproc \
		-lcrypto -lpthread -lm -lstdc++

OBJDIR = obj.$(SYSTEM).$(OPT).$(CXX)
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)%.cpp,$(OBJDIR)%.o,$(SRC))
$(shell if [ ! -d ${OBJDIR} ]; then mkdir ${OBJDIR}; fi)

ifeq ($(CXX), g++)
	GCC = /usr/bin/g++
endif

ifeq ($(OPT), release)
	CPPFLAGS = -c -fpic -O3 -DNDEBUG  -DPPLM_FLOAT -DPYTHON_HEAD_FILE_INCLUDE
endif
ifeq ($(OPT), debug)
	CPPFLAGS = -c -fpic -Ddebug -DNDEBUG  -DPPLM_FLOAT -DPYTHON_HEAD_FILE_INCLUDE
endif

# -------------------------------------------------
# Code generation
# -------------------------------------------------
$(TARGET) : ${OBJ}
	$(GCC) -g -o $@ $^ $(LIBDIR) $(INCDIR)

${OBJDIR}/%.o : $(SRCDIR)/%.cpp
	$(GCC) $(CPPFLAGS) -g -c $< -o $@ $(INCDIR)

clean:
	rm -rf ${OBJDIR} $(TARGET)
