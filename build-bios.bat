@echo off
setlocal 

REM Check if image 'bios_build_env' exists
docker image inspect bios_build_env >nul 2>&1
if errorlevel 1 (
    echo Image not found. Building...
    docker build -t bios_build_env ./docker/
) else (
    echo Image already exists. Skipping build...
)

docker run -v.:/base -w /base bios_build_env /bin/bash -i -c "/base/bios/build.sh /base/build/ROMs /base/workspace/ROMs"
docker container prune -f