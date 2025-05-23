ARG inst_dir="/tmp/out"
ARG image_name
FROM nap-${image_name} AS builder
ARG inst_dir

ENV src_dir="/input"
COPY source ${src_dir}

# Configure and make openssl
RUN \
  cd ${src_dir} && \
  ./Configure --prefix=${inst_dir} && \
  make -j `nproc` && \
  make -j `nproc` install && \
  cp README.md ${inst_dir} && \
  rm -rf build ${src_dir}

# rename lib64 to lib if necessary, rename lib to libs
RUN \
  /bin/bash -c "if [ -d ${inst_dir}/lib64 ]; then mv ${inst_dir}/lib64 ${inst_dir}/lib; fi" && \
  /bin/bash -c "if [ -d ${inst_dir}/lib ]; then mv ${inst_dir}/lib ${inst_dir}/libs; fi"

# delete unneccessary dirs and files
RUN \
  rm ${inst_dir}/libs/*a && \
  rm -rf ${inst_dir}/bin ${inst_dir}/include ${inst_dir}/include ${inst_dir}/ssl ${inst_dir}/share && \
  rm -rf ${inst_dir}/libs/engines-3 ${inst_dir}/libs/ossl-modules ${inst_dir}/libs/pkgconfig && \
  rm ${inst_dir}/README.md

# Copy the libs dir from builder to current working dir
FROM scratch
ARG inst_dir
COPY --from=builder ${inst_dir} /
