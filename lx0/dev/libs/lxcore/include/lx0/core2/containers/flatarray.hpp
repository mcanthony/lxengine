//===========================================================================//
/*
                                   LxEngine

    LICENSE

    Copyright (c) 2012 athile@athile.net (http://www.athile.net)

    Permission is hereby granted, free of charge, to any person obtaining a 
    copy of this software and associated documentation files (the "Software"), 
    to deal in the Software without restriction, including without limitation 
    the rights to use, copy, modify, merge, publish, distribute, sublicense, 
    and/or sell copies of the Software, and to permit persons to whom the 
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
    IN THE SOFTWARE.
*/
//===========================================================================//

#pragma once

namespace lx0
{
    namespace core2
    {
        namespace containers_ns
        {
            //===========================================================================//
            //! A "vector-of-vectors" laid out in a single contiguous memory block
            /*!
                \ingroup lx0_core2_containers
             */
            /*!
                A single, contiguous data array laid out as follows
                [index ........... ] for all N elements
                [count data data] [count data] [count data data data]

                MUCH faster than a std::vector<std::vector<>>.
             */
            template <typename ElemType>
            class flat_vector_array_t
            {
            public:
                flat_vector_array_t     () : m_array(0) { }
                ~flat_vector_array_t    () { delete[] m_array; }

                void set (int size, int* arr) { m_size = size; m_array = arr; }
                void clear () { delete[] m_array; m_array = 0; m_size = 0; }
                bool empty() const { return m_array == 0; }

                int  listSize ()                 const { return m_size; }
                int  elemSize  (int elem)        const { return m_array[ m_array[elem] ]; }
                int  elem      (int elem, int i) const { return m_array[ m_array[elem] + i + 1]; }

                int* alloc    (int size, int dataSize)
                {
                    int total = 2 * size + dataSize;
                    m_size = size;
                    m_array = new int[total];
                    ::memset(m_array, 0, sizeof(int) * total);
                    return m_array;
                }
                void initElemSizes (std::vector<int>& sizes)
                {
                    int index = m_size;        
                    for (int elem = 0; elem < m_size; ++elem)
                    {
                        m_array[elem] = index;        // index to count/data segment
                        m_array[index] = 0;           // initial count = 0
                        index += sizes[elem] + 1;     // 1 slot for the count itself
                    } 
                }
                void appendElemValue (int elem, int value)
                {
                    int  countIndex = m_array[elem];
                    int& count      = m_array[countIndex];
                    count++;
                    m_array[countIndex + count] = int(value);
                }

            protected:
                int             m_size;
                int*            m_array;
            };

            
        }
    }

    using namespace core2::containers_ns;
}
