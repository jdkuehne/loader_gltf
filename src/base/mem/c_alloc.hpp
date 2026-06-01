#ifndef JK_C_ALLOC_H
#define JK_C_ALLOC_H

namespace base::mem
{

void *cstd_malloc(U64 size);
void cstd_free(void *memory);

}

#endif
