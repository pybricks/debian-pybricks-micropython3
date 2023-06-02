#!/bin/sh

set -e

# debian version - be sure to include `~` in beta versions!
version="3.3.0~b6"

# note: mk-origtargz filters files based on Files-Excluded in ./copyright

wget https://github.com/pybricks/pybricks-micropython/archive/refs/tags/v3.3.0b6.tar.gz -O ../pybricks-micropython_${version}.orig.tar.gz
mk-origtargz --rename ../pybricks-micropython_${version}.orig.tar.gz

wget https://github.com/pybricks/micropython/archive/refs/tags/v1.20.0-23-g6c633a8dd.tar.gz -O ../pybricks-micropython_${version}.orig-micropython.tar.gz
mk-origtargz --rename --component micropython ../pybricks-micropython_${version}.orig-micropython.tar.gz

wget https://github.com/micropython/axtls/archive/531cab9c278c947d268bd4c94ecab9153a961b43.tar.gz -O ../pybricks-micropython_${version}.orig-axtls.tar.gz
mk-origtargz --repack --rename --component axtls ../pybricks-micropython_${version}.orig-axtls.tar.gz

wget https://github.com/pfalcon/berkeley-db-1.xx/archive/35aaec4418ad78628a3b935885dd189d41ce779b.tar.gz -O ../pybricks-micropython_${version}.orig-berkeley-db.tar.gz
mk-origtargz --repack --rename --component berkeley-db ../pybricks-micropython_${version}.orig-berkeley-db.tar.gz

wget https://github.com/micropython/micropython-lib/archive/c113611765278b2fc8dcf8b2f2c3513b35a69b39.tar.gz -O ../pybricks-micropython_${version}.orig-micropython-lib.tar.gz
mk-origtargz --repack --rename --component micropython-lib ../pybricks-micropython_${version}.orig-micropython-lib.tar.gz

gbp import-orig --component=micropython --component=axtls --component=berkeley-db --component=micropython-lib --upstream-version=${version} ../pybricks-micropython_${version}.orig.tar.xz
