# Copyright 2020 Artem Y. Polyakov <artpol84@gmail.com>
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors may
# be used to endorse or promote products derived from this software without specific
# prior written permission.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#!/bin/bash

ulimit -c unlimited

function exec_test()
{
    if [ -f "./$1" ]; then
        execs="./$1"
    else
        execs="./${1}_offs0 ./${1}_offsM ./${1}_rcvdt_exp ${1}_rcvdt_unexp "
    fi
    for e in $execs; do
	if [ ! -f "$e" ]; then
	    continue
	fi
#	    DBGFLAGS="-x ARTPOL_USE_HW=1 -x ARTPOL_DEBUG_RANK=1 "
        cmd="$MPIRUN -np 2 --map-by node --mca pml ucx -x UCX_TLS=rc_x --mca btl self -x OMPI_WANT_UMR=1 $DBGFLAGS $e"
        echo "Executing: $cmd"
        $cmd || exit 1
    done
}

tests="vector vector_cnt vector_cnt2 vector_rcache vector_2d index_plain index_regular_str index_regular_ilv index_ilv_w_block index_2x_ilv index_2x_str index_mixed1 index_two_dts index_zcopy_multi index_repro_bug1 dt_create_overhead"
#tests="dt_create_overhead"
#tests="vector 9index_plain index_regular_str index_regular_ilv index_ilv_w_block index_2x_ilv index_2x_str index_mixed1 index_two_dts index_zcopy_multi"

if [ -z "$MPIRUN" ]; then
    export MPIRUN=mpirun
fi

for t in $tests; do
    exec_test $t
done

