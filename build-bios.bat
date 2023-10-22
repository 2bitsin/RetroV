docker build -t bios_build_env ./docker/
docker run -v.:/base -it bios_build_env /bin/bash -c "cd /base/bios && ./build.sh /base/build/bios /base/workspace/ROMs"
