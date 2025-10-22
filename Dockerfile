# ============================================================
# Dockerfile for Graph Optimizer Beta Testing
# ============================================================

ARG USE_GPU=false
ARG BASE_IMAGE=ubuntu:24.04
FROM ${BASE_IMAGE} AS builder
ARG USE_GPU

ENV DEBIAN_FRONTEND=noninteractive \
    LANG=C.UTF-8 \
    LC_ALL=C.UTF-8 \
    PYTHONDONTWRITEBYTECODE=1

ENV LD_LIBRARY_PATH=/workspace/lib:$LD_LIBRARY_PATH

WORKDIR /workspace

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    make \
    g++ \
    clang-15 \
    libfmt-dev \
    python3 \
    python3-pip \
    python3-venv \
    git \
    ca-certificates \
    wget \
    curl \
    libomp-dev \
    jupyter-notebook \
    libgraphblas-dev \
    vim \
    unzip \
 && rm -rf /var/lib/apt/lists/*

COPY . .

RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"
RUN pip install --upgrade pip setuptools wheel && \
    pip install -r requirements.txt

RUN unzip -d ./data/beta_testing/large ./data/beta_testing/higgs_social_network.zip || true

RUN chmod +x ./make_all.sh && \
    if [ "$USE_GPU" = "true" ]; then \
        echo "Building with GPU support"; \
        ./make_all.sh --include-gpu; \
    else \
        echo "Building CPU-only"; \
        ./make_all.sh; \
    fi

EXPOSE 7777
ENV LD_LIBRARY_PATH=/workspace:${LD_LIBRARY_PATH}

CMD ["bash", "-c", "cd /workspace/beta_testing && jupyter notebook --ip=0.0.0.0 --port=7777 --no-browser --allow-root --NotebookApp.token=''"]
