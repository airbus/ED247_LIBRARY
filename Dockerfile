FROM centos:centos7
WORKDIR /ED247_LIBRARY
COPY . /ED247_LIBRARY

# Build and install dependencies
RUN yum -y install git wget git devtoolset-7 gcc-c++ flex bison autoconf libtool make automake python-devel perl-Digest-MD5
RUN chmod +x contrib/cmake-3.13.4-Linux-x86_64.sh
RUN contrib/cmake-3.13.4-Linux-x86_64.sh --skip-license --prefix=/usr
RUN cd contrib/ninja-v1.8.2 ; ./configure.py --bootstrap ; cp ninja /usr/bin
RUN cd contrib/lcov-v1.13 ; make install
RUN cd contrib/doxygen-v1.8.11 ; mkdir build ; cd build ; cmake -G "Unix Makefiles" .. ; make ; make install
RUN cd contrib/googletest-v1.8.1 ; mkdir build ; cd build ; cmake .. ; make ; make install
RUN cd contrib/libxml2-v2.9.9 ; ./autogen.sh ; make ; make install 

# Build the ED-247 library
RUN sed -i 's/To be filled with CMake (V3.13.4) root path/\/usr\/bin/g' dependencies.sh
RUN sed -i 's/To be filled with Ninja (V1.8.2) root path/\/usr\/bin/g' dependencies.sh
RUN sed -i 's/To be filled with Lcov (V1.13.0) root path/\/usr\/local\/bin/g' dependencies.sh
RUN sed -i 's/To be filled with Doxygen (V1.8.11) root path/\/usr\/local\/bin/g' dependencies.sh
RUN sed -i 's/To be filled with GTest (V1.8.1) root path/\/usr\/local\/lib64\/cmake\/GTest/g' dependencies.sh
RUN sed -i 's/To be filled with LIBXML2 (V2.9.9) root path/\/usr\/local/g' dependencies.sh
RUN chmod +x mgr.sh
RUN export CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH:/usr/local/lib64/cmake/GTest
RUN ./mgr.sh setup gcc4.8.5 x64
RUN ./mgr.sh build gcc4.8.5 x64
RUN ./mgr.sh tests gcc4.8.5 x64
