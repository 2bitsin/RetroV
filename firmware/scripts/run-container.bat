@echo off
docker run -v %CD%:/var/root -it firmware-build:latest
docker container prune -f