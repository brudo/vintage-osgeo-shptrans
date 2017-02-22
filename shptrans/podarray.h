/** 
 * podarray.h - published as part of SHPTRANS
 *
 *
 * SHPTRANS is Copyright (c) 1999-2004 Bruce Dodson and others.
 * All rights Reserved.
 * 
 * Permission to use, copy, modify, merge, publish, perform,
 * distribute, sublicense, and/or sell copies of this original work
 * of authorship (the "Software") and derivative works thereof, is 
 * hereby granted free of charge to any person obtaining a copy of 
 * the Software, subject to the following conditions:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimers.
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimers in 
 *    the documentation and/or other materials provided with the 
 *    distribution.
 *
 * 3. Neither the names of the copyright holders, nor the names of any
 *    contributing authors, may be used to endorse or promote products
 *    derived from the Software without specific prior written
 *    permission.
 *
 * 4. If you modify a copy of the Software, or any portion thereof,
 *    you must cause the modified files to carry prominent notices 
 *    stating that you changed the files.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND.
 * THE COPYRIGHT HOLDERS AND CONTRIBUTING AUTHORS DISCLAIM ANY AND
 * ALL WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 * A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTING AUTHORS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING IN ANY WAY OUT OF THE USE
 * OR DISTRIBUTION OF THE SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
**/


// pod_array is an auto-growing array which can hold only plain ol' data
// since it uses realloc.

#ifndef PODARRAY_H
#define PODARRAY_H


extern "C" {
    typedef unsigned size_t;
    void *realloc(void*, size_t);
}

template <class elt_t, unsigned ncolumns> class pod_array {
  public:
    pod_array(size_t sz = 0):pArray(0),size(0) { resize(sz); }
    ~pod_array() { resize(0); }
  
    int resize(size_t newsize); //manually resizes the array.
    size_t storage() const { return size; }
  
    elt_t* operator[](size_t offset); //resizes if needed
    elt_t const * operator[](size_t offset) const;
    elt_t* reserve(size_t need_size);
    elt_t* detach() { elt_t *ptr = pArray; pArray = NULL; size = 0; return ptr; }
  private:
    elt_t* pArray;
    size_t size;
  
    //not implemented:
    pod_array(const pod_array&);
    void operator=(const pod_array&);
};

template <class elt_t, unsigned ncolumns>
int pod_array<elt_t,ncolumns>::resize(size_t newsize) {
    elt_t * newArray = (elt_t*)realloc(pArray, newsize * sizeof(elt_t) * (ncolumns?ncolumns:1));
    if (newArray) {
        pArray = newArray;
        size = newsize;
        return 1;
    } else if (!newsize) {
        pArray = 0;
        size = 0;
        return 1;
    } else {
        return 0;
    }
}

template <class elt_t, unsigned ncolumns>
elt_t * pod_array<elt_t,ncolumns>::reserve(size_t needsize) {
    if (needsize>size) {
        size_t newsize = (size+1)*2;
        while (needsize>newsize) newsize *=2;
        return resize(newsize) ? pArray : (elt_t*)0;
    }
    return pArray;
}


template <class elt_t, unsigned ncolumns>
elt_t * pod_array<elt_t,ncolumns>::operator[](size_t offset) {
    if (offset>=size) {
        size_t newsize = (size+1)*2;
        while (offset>=newsize) newsize *=2;
    
        if (!resize(newsize)) {
            return (elt_t*)0;
        }
    }
  
    return pArray + (offset * (ncolumns?ncolumns:1));
}


template <class elt_t, unsigned ncolumns>
elt_t const * pod_array<elt_t,ncolumns>::operator[](size_t offset) const {
    if (offset>=size) return (elt_t const*)0;
    return (elt_t const *)(pArray + (offset * (ncolumns?ncolumns:1)));
}

#endif

