sudo docker buildx build --platform linux/amd64 -t dbartutw/graph-optimizer-beta-testing:latest --push .
sudo docker buildx build --build-arg USE_GPU=true --build-arg BASE_IMAGE=nvidia/cuda:12.2.0-devel-ubuntu22.04 -t dbartutw/graph-optimizer-beta-testing-gpu:latest --push .
