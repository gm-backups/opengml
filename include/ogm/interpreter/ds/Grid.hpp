#pragma once

#include "ogm/interpreter/Variable.hpp"

#include <vector>

namespace ogm { namespace interpreter
{
    struct DSGrid
    {
        std::vector<std::vector<Variable>> m_data; // indexed column-row
        size_t m_width;
        size_t m_height;

        DSGrid(size_t width, size_t height)
            : m_width(width)
            , m_height(height)
        { }
        
        #ifdef OGM_GARBAGE_COLLECTOR
        void ds_integrity_check()
        {
            for (auto& vec : m_data)
            {
                for (Variable& v : vec)
                {
                    v.gc_integrity_check();
                }
            }
        }
        #endif
    };
}}
