sudo docker build -t graph-optimizer-beta-testing .
sudo docker run -p 7777:7777 -it --privileged graph-optimizer-beta-testing
