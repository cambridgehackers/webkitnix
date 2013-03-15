/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 * Copyright (c) 2010 University of Szeged
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "config.h"
#if USE(UNIX_DOMAIN_SOCKETS)
#include "SharedMemory.h"

#include "ArgumentDecoder.h"
#include "ArgumentEncoder.h"
#include "WebCoreArgumentCoders.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wtf/Assertions.h>
#include <wtf/CurrentTime.h>
#include <wtf/RandomNumber.h>
#include <wtf/text/CString.h>

#include <linux/ashmem.h>

#define ASHMEM_DEVICE	"/"ASHMEM_NAME_DEF


namespace WebKit {

SharedMemory::Handle::Handle()
    : m_fileDescriptor(-1)
    , m_size(0)
{
    fprintf(stderr,"SharedMemory::Handle\n");
}

SharedMemory::Handle::~Handle()
{
    fprintf(stderr,"SharedMemory::Handle destructor\n");
    if (!isNull())
        while (close(m_fileDescriptor) == -1 && errno == EINTR) { }
}

bool SharedMemory::Handle::isNull() const
{
    return m_fileDescriptor == -1;
}

void SharedMemory::Handle::encode(CoreIPC::ArgumentEncoder& encoder) const
{
    fprintf(stderr,"SharedMemory::Handle encode\n");
    encoder.encode(releaseToAttachment());
}

bool SharedMemory::Handle::decode(CoreIPC::ArgumentDecoder* decoder, Handle& handle)
{
    fprintf(stderr,"SharedMemory::Handle decode\n");
    ASSERT_ARG(handle, !handle.m_size);
    ASSERT_ARG(handle, handle.isNull());

    CoreIPC::Attachment attachment;
    if (!decoder->decode(attachment))
        return false;

    handle.adoptFromAttachment(attachment.releaseFileDescriptor(), attachment.size());
    return true;
}

CoreIPC::Attachment SharedMemory::Handle::releaseToAttachment() const
{
    fprintf(stderr,"SharedMemory::Handle releasetoattachment\n");
    int temp = m_fileDescriptor;
    m_fileDescriptor = -1;
    return CoreIPC::Attachment(temp, m_size);
}

void SharedMemory::Handle::adoptFromAttachment(int fileDescriptor, size_t size)
{
    fprintf(stderr,"SharedMemory::Handle adoptfromattachment fd=%d size=%d \n",fileDescriptor,size);
    ASSERT(!m_size);
    ASSERT(isNull());

    m_fileDescriptor = fileDescriptor;
    m_size = size;
}

/*
 * ashmem_create_region - creates a new ashmem region and returns the file
 * descriptor, or <0 on error
 *
 * `name' is an optional label to give the region (visible in /proc/pid/maps)
 * `size' is the size of the region, in page-aligned bytes
 */
int ashmem_create_region(const char *name, size_t size)
{
	int fd, ret;
        fprintf(stderr,"ashmem_create_region\n",name,size);
	fd = open(ASHMEM_DEVICE, O_RDWR);
	if (fd < 0)
		return fd;

	if (name) {
		char buf[ASHMEM_NAME_LEN];

		strlcpy(buf, name, sizeof(buf));
		ret = ioctl(fd, ASHMEM_SET_NAME, buf);
		if (ret < 0)
			goto error;
	}

	ret = ioctl(fd, ASHMEM_SET_SIZE, size);
	if (ret < 0)
		goto error;

	return fd;

error:
	close(fd);
	return ret;
}
int ashmem_set_prot_region(int fd, int prot)
{
    fprintf(stderr,"SharedMemory::set_prot fd=%d prot=%d\n",fd,prot);
    return ioctl(fd, ASHMEM_SET_PROT_MASK, prot);
}


PassRefPtr<SharedMemory> SharedMemory::create(size_t size)
{
    CString tempName;
    fprintf(stderr,"SharedMemory::create called for size=%d\n",size);
    int fileDescriptor = -1;
#if 0
    for (int tries = 0; fileDescriptor == -1 && tries < 10; ++tries) {
        String name = String("/WK2SharedMemory.") + String::number(static_cast<unsigned>(WTF::randomNumber() * (std::numeric_limits<unsigned>::max() + 1.0)));
        tempName = name.utf8();

        do {
            fileDescriptor = shm_open(tempName.data(), O_CREAT | O_CLOEXEC | O_RDWR, S_IRUSR | S_IWUSR);
        } while (fileDescriptor == -1 && errno == EINTR);
    }
#endif
    String name = String("/WK2SharedMemory.") + String::number(static_cast<unsigned>(WTF::randomNumber() * (std::numeric_limits<unsigned>::max() + 1.0)));
    tempName = name.utf8();
    
    fileDescriptor =ashmem_create_region(tempName.data(),size);
    if (fileDescriptor == -1)
        return 0;

#if 0
    while (ftruncate(fileDescriptor, size) == -1) {
        if (errno != EINTR) {
            while (close(fileDescriptor) == -1 && errno == EINTR) { }
            shm_unlink(tempName.data());
            return 0;
        }
    }
#endif

    void* data = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
    if (data == MAP_FAILED) {
        //while (close(fileDescriptor) == -1 && errno == EINTR) { }
        //shm_unlink(tempName.data());
        fprintf(stderr,"SharedMemoryAshmem.cpp mmap failed!\n");
        return 0;
    }

    //XXX mem leak introduced here?
    //shm_unlink(tempName.data());

    RefPtr<SharedMemory> instance = adoptRef(new SharedMemory());
    instance->m_data = data;
    instance->m_fileDescriptor = fileDescriptor;
    instance->m_size = size;
    return instance.release();
}

static inline int accessModeMMap(SharedMemory::Protection protection)
{

    switch (protection) {
    case SharedMemory::ReadOnly:
        return PROT_READ;
    case SharedMemory::ReadWrite:
        return PROT_READ | PROT_WRITE;
    }

    ASSERT_NOT_REACHED();
    return PROT_READ | PROT_WRITE;
}

PassRefPtr<SharedMemory> SharedMemory::create(const Handle& handle, Protection protection)
{

    fprintf(stderr,"SharedMemory::create by handle \n");
    ASSERT(!handle.isNull());

    void* data = mmap(0, handle.m_size, accessModeMMap(protection), MAP_SHARED, handle.m_fileDescriptor, 0);
    if (data == MAP_FAILED)
        return 0;

    RefPtr<SharedMemory> instance = adoptRef(new SharedMemory());
    instance->m_data = data;
    instance->m_fileDescriptor = handle.m_fileDescriptor;
    instance->m_size = handle.m_size;
    handle.m_fileDescriptor = -1;
    return instance;
}

SharedMemory::~SharedMemory()
{
    fprintf(stderr,"SharedMemory::destructor \n");
    munmap(m_data, m_size);
    while (close(m_fileDescriptor) == -1 && errno == EINTR) { }
}

static inline int accessModeFile(SharedMemory::Protection protection)
{
    switch (protection) {
    case SharedMemory::ReadOnly:
        return O_RDONLY;
    case SharedMemory::ReadWrite:
        return O_RDWR;
    }

    ASSERT_NOT_REACHED();
    return O_RDWR;
}

bool SharedMemory::createHandle(Handle& handle, Protection protection)
{
    fprintf(stderr,"SharedMemory::createHandle by handle \n");
    ASSERT_ARG(handle, !handle.m_size);
    ASSERT_ARG(handle, handle.isNull());

    int duplicatedHandle;
    while ((duplicatedHandle = dup(m_fileDescriptor)) == -1) {
        if (errno != EINTR) {
            ASSERT_NOT_REACHED();
            return false;
        }
    }

    while ((fcntl(duplicatedHandle, F_SETFD, FD_CLOEXEC | accessModeFile(protection)) == -1)) {
        if (errno != EINTR) {
            ASSERT_NOT_REACHED();
            while (close(duplicatedHandle) == -1 && errno == EINTR) { }
            return false;
        }
    }
    handle.m_fileDescriptor = duplicatedHandle;
    handle.m_size = m_size;
    return true;
}

unsigned SharedMemory::systemPageSize()
{
    fprintf(stderr,"SharedMemory::systemPageSize called \n");
    static unsigned pageSize = 0;

    if (!pageSize)
        pageSize = getpagesize();
    fprintf(stderr,"SharedMemory::systemPageSize = %d\n",pageSize);
    return pageSize;
}

} // namespace WebKit

#endif

