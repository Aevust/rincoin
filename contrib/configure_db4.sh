#!/bin/bash
export BDB_PREFIX='/home/tomas_admin/rincoin/db4'
./configure BDB_LIBS="-L${BDB_PREFIX}/lib -ldb_cxx-4.8" BDB_CFLAGS="-I${BDB_PREFIX}/include" "$@"