@echo off
docker build -t firmware-build:latest .
docker run -v .:/var/root -it firmware-build:latest