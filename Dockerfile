# ============================================================
# Dockerfile for Graph Optimizer Beta Testing
# ============================================================

# --- 1. Choose base image ---
# If GPU is enabled -> use CUDA dev image (with nvcc)
# Else -> use plain Ubuntu
ARG USE_GPU=true
FROM ubuntu:24.04 AS base

# If GPU support requested, switch to CUDA base image
FROM nvidia/cuda:12.2.0-devel-ubuntu22.04 AS cuda-base
FROM base AS final-base
FROM ${USE_GPU:+cuda-base} AS builder

# --- 2. Setup system environment ---
ENV DEBIAN_FRONTEND=noninteractive \
    LANG=C.UTF-8 \
    LC_ALL=C.UTF-8 \
    PYTHONDONTWRITEBYTECODE=1

ENV LD_LIBRARY_PATH=/workspace/lib:$LD_LIBRARY_PATH

WORKDIR /workspace

# --- 3. Install dependencies ---
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
 && rm -rf /var/lib/apt/lists/*

# --- 4. Copy your project ---
COPY . .

# --- 5. Setup Python environment ---
RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"
RUN pip install --upgrade pip setuptools wheel && \
    pip install -r requirements.txt

# --- 6. Build all CPU/GPU targets ---
RUN chmod +x ./make_all.sh && \
    ./make_all.sh --include-gpu;

# --- 7. Setup runtime environment ---
EXPOSE 7777
ENV LD_LIBRARY_PATH=/workspace:${LD_LIBRARY_PATH}

# --- 8. Default command ---
# Launch Jupyter from the beta_testing directory
CMD ["bash", "-c", "cd /workspace/beta_testing && jupyter notebook --ip=0.0.0.0 --port=7777 --no-browser --allow-root --NotebookApp.token=''"]
