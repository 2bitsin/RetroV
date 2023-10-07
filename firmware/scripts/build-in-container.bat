@echo off
setlocal enabledelayedexpansion
set WORKSPACE=%CD%\..\workspace\ROMs
docker run -v %CD%:/var/root -v %WORKSPACE%:/var/install firmware-build:latest /bin/bash -c "cd /var/root && ./scripts/build.sh"
docker container prune -f
