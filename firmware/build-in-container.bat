setlocal enabledelayedexpansion
set WORKSPACE=%CD%\..\workspace\ROMs
docker run -v .:/var/root -v %WORKSPACE%:/var/install firmware-build:latest /bin/bash -c "cd /var/root && ./scripts/build.sh"
