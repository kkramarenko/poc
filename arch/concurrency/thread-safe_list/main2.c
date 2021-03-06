#include "common.h"
#include "tslist.h"
#include "arch.h"


pthread_barrier_t tbarrier;
tslist_t *list;

int global_thread_count = -1;
__thread int thread_id = -1;


typedef struct
{
    int thread, seqn;
} data_t;

void *producer(void *tmp)
{
    data_t *data = calloc(nadds * nbatch, sizeof(data_t));
    void *ptrs[nbatch];
    int i, j;

    thread_id = atomic_inc(&global_thread_count, 1);

    bind_to_core(thread_id);

    pthread_barrier_wait(&tbarrier);

    for(i=0; i < nadds; i++) {
        for(j = 0; j < nbatch; j++) {
            int idx = i * nbatch + j;
            data[idx].thread = thread_id;
            data[idx].seqn = i*nbatch + j;
            ptrs[j] = &data[idx];
        }
        tslist_append_batch(list, ptrs, nbatch);
    }
}

void *consumer(void *tmp)
{
    int count[nthreads];
    int prev[nthreads];
    int i, gcount = 0;


    for(i = 0; i < nthreads; i++) {
        count[i] = 0;
        prev[i] = -1;
    }

    /* Bind to the last core */
    bind_to_core(nthreads);

    pthread_barrier_wait(&tbarrier);

    while( gcount < nthreads * nadds * nbatch ) {
        tslist_elem_t *head;
        tslist_dequeue(list, &head);
        while( head ){
            data_t *data = (data_t *)head->ptr;
            if( (prev[data->thread] + 1) != data->seqn ){
                printf("Error: sequence from thread #%d: %d vs %d, gcount = %d\n",
                       data->thread, prev[data->thread], data->seqn, gcount );
            }
            count[data->thread]++;
            prev[data->thread] = data->seqn;
            gcount++;
            head = head->next;
        }
    }
}

int main(int argc, char **argv)
{
    int i;
    pthread_t *id;
    double start, time = 0;

    process_args(argc,argv);
//    bind_to_core(0);

    pthread_barrier_init(&tbarrier, NULL, nthreads + 2);

    id = calloc(nthreads + 1, sizeof(*id));
    list = tslist_create(nthreads, nadds * nbatch);

    /* setup and create threads */
    pthread_create(&id[nthreads], NULL, consumer, (void *)NULL);
    for (i=0; i<nthreads; i++) {
        pthread_create(&id[i], NULL, producer, (void *)NULL);
    }

    pthread_barrier_wait(&tbarrier);
    start = GET_TS();

    for (i=0; i<nthreads + 1; i++) {
        pthread_join(id[i], NULL);
    }
    time = GET_TS() - start;
    int count = nthreads * nadds * nbatch;
    printf("Number of elements is %d\n", count);
    printf("Time: %lf\n", time * 1E6);
    printf("Performance: %lf Melem/s\n", count / time / 1E6);

    tslist_append_done(list, nthreads);
}

/*
Copyright 2019 Artem Y. Polyakov <artpol84@gmail.com>
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may
be used to endorse or promote products derived from this software without specific
prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.
*/
