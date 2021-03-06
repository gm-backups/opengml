#include "libpre.h"
    #include "fn_buffer.h"
#include "libpost.h"

#include "ogm/interpreter/Variable.hpp"
#include "ogm/common/error.hpp"
#include "ogm/common/util.hpp"
#include "ogm/interpreter/ds/List.hpp"
#include "ogm/interpreter/Executor.hpp"
#include "serialize_g.hpp"
#include "ogm/common/error.hpp"

#include <string>
#include <cctype>
#include <cstdlib>
#include <sstream>

using namespace ogm::interpreter;
using namespace ogm::interpreter::fn;

#define frame staticExecutor.m_frame

typedef size_t buffer_type_t;
const buffer_type_t
    k_u8 = 0,
    k_s8 = 1,
    k_u16 = 2,
    k_s16 = 3,
    k_u32 = 4,
    k_s32 = 5,
    k_u64 = 6,
    k_f16 = 7,
    k_f32 = 8,
    k_f64 = 9,
    k_bool = 10,
    k_string = 11,
    k_text = 12;

void ogm::interpreter::fn::buffer_create(VO out, V size, V type, V alignment)
{
    out = static_cast<real_t>(
        frame.m_buffers.create_buffer(
            size.castCoerce<size_t>(),
            static_cast<Buffer::Type>(type.castCoerce<int32_t>()),
            alignment.castCoerce<size_t>()
        )
    );
}

void ogm::interpreter::fn::buffer_delete(VO out, V id)
{
    frame.m_buffers.delete_buffer(id.castCoerce<size_t>());
}

void ogm::interpreter::fn::buffer_exists(VO out, V id)
{
    out = frame.m_buffers.buffer_exists(id.castCoerce<size_t>());
}

void ogm::interpreter::fn::buffer_get_alignment(VO out, V id)
{
    Buffer& b = frame.m_buffers.get_buffer(id.castCoerce<size_t>());
    out = b.get_align();
}

void ogm::interpreter::fn::buffer_get_type(VO out, V id)
{
    Buffer& b = frame.m_buffers.get_buffer(id.castCoerce<size_t>());
    out = b.get_type();
}

void ogm::interpreter::fn::buffer_sizeof(VO out, V type)
{
    int32_t size = [](int32_t type) -> int
    {
        switch(type)
        {
        case k_u8:
            return 1;
        case k_s8:
            return 1;
        case k_u16:
            return 2;
        case k_s16:
            return 2;
        case k_u32:
            return 4;
        case k_s32:
            return 4;
        case k_u64:
            return 8;
        case k_f16:
            return 2;
        case k_f32:
            return 4;
        case k_f64:
            return 8;
        case k_bool:
            return 1;
        case k_string:
            // TODO
            return 0;
        case k_text:
            // TODO
            return 0;
        default:
            return 0;
        }
    }
    (type.castCoerce<buffer_type_t>());
    
    out = static_cast<real_t>(size);
}

void ogm::interpreter::fn::buffer_read(VO out, V id, V type)
{
    Buffer& b = frame.m_buffers.get_buffer(id.castCoerce<size_t>());
    auto t = type.castCoerce<buffer_type_t>();
    switch(t)
    {
    case k_u8:
        out = static_cast<int32_t>(b.read<unsigned char>());
        break;
    case k_s8:
        out = static_cast<int32_t>(b.read<char>());
        break;
    case k_u16:
        out = b.read<uint16_t>();
        break;
    case k_s16:
        out = b.read<int16_t>();
        break;
    case k_u32:
        out = b.read<uint32_t>();
        break;
    case k_s32:
        out = b.read<int32_t>();
        break;
    case k_u64:
        out = b.read<uint64_t>();
        break;
    case k_f16:
        throw MiscError("float16 not supported.");
        break;
    case k_f32:
        out = b.read<float>();
        break;
    case k_f64:
        out = b.read<double>();
        break;
    case k_bool:
        out = !!b.read<uint8_t>();
        break;
    case k_string:
    case k_text:
        {
            std::stringstream ss;
            while (!b.eofr())
            {
                const char c = b.read<char>();
                if (c || t == k_text)
                {
                    ss << c;
                }
                else
                {
                    break;
                }
            }
            out = ss.str();
        }
        break;
    }
}

void ogm::interpreter::fn::buffer_write(VO out, V id, V type, V value)
{
    Buffer& b = frame.m_buffers.get_buffer(id.castCoerce<size_t>());
    switch(type.castCoerce<buffer_type_t>())
    {
    case k_u8:
        b.write(static_cast<uint8_t>(value.castCoerce<int32_t>()));
        break;
    case k_s8:
        b.write(static_cast<int8_t>(value.castCoerce<int32_t>()));
        break;
    case k_u16:
        b.write(static_cast<uint16_t>(value.castCoerce<int32_t>()));
        break;
    case k_s16:
        b.write(static_cast<int16_t>(value.castCoerce<int32_t>()));
        break;
    case k_u32:
        b.write(value.castCoerce<uint32_t>());
        break;
    case k_s32:
        b.write(value.castCoerce<int32_t>());
        break;
    case k_u64:
        b.write(value.castCoerce<uint64_t>());
        break;
    case k_f16:
        throw MiscError("float16 not supported.");
        break;
    case k_f32:
        b.write(static_cast<float>(value.castCoerce<real_t>()));
        break;
    case k_f64:
        b.write(static_cast<double>(value.castCoerce<real_t>()));
        break;
    case k_bool:
        b.write(!!static_cast<uint8_t>(value.cond()));
        break;
    case k_string:
        {
            std::string_view s = value.castCoerce<std::string_view>();
            for (size_t i = 0; i < s.length(); ++i)
            {
                b.write(static_cast<int8_t>(s[i]));
            }
            b.write(static_cast<int8_t>(0));
        }
    case k_text:
        {
            std::string_view s = value.castCoerce<std::string_view>();
            for (size_t i = 0; i < s.length(); ++i)
            {
                b.write(static_cast<int8_t>(s[i]));
            }
        }
        break;
    }
}

void ogm::interpreter::fn::buffer_tell(VO out, V id)
{
    Buffer& b = frame.m_buffers.get_buffer(id.castCoerce<size_t>());
    out = static_cast<real_t>(b.tell());
}

void ogm::interpreter::fn::buffer_seek(VO out, V id, V vbase, V voffset)
{
    Buffer& b = frame.m_buffers.get_buffer(id.castCoerce<size_t>());
    int32_t o = voffset.castCoerce<int32_t>();
    size_t prev = b.tell();
    const int32_t base = vbase.castCoerce<int32_t>();
    switch(base)
    {
    case 0: // start
        b.seek(o);
        break;
    case 1: // relative
        b.seek(prev + o);
        break;
    case 2: // end
        b.seek(b.size() + o);
        break;
    default:
        throw MiscError("buffer seek base not recognized");
    }
}

void ogm::interpreter::fn::buffer_peek(VO out, V id, V offset, V type)
{
    Buffer& b = frame.m_buffers.get_buffer(id.castCoerce<size_t>());
    size_t prev = b.tell();
    b.seek(offset.castCoerce<size_t>());
    buffer_read(out, id, type);
    b.seek(prev);
}

void ogm::interpreter::fn::buffer_poke(VO out, V id, V offset, V type, V value)
{
    Buffer& b = frame.m_buffers.get_buffer(id.castCoerce<size_t>());
    size_t prev = b.tell();
    b.seek(offset.castCoerce<size_t>());
    buffer_write(out, id, type, value);
    b.seek(prev);
}

void ogm::interpreter::fn::buffer_fill(
    VO out, V id, V offset, V type, V value, V size
)
{
    Buffer& b = frame.m_buffers.get_buffer(id.castCoerce<size_t>());
    size_t pre = b.tell();
    size_t begin = offset.castCoerce<size_t>();
    size_t end = begin + size.castCoerce<size_t>();
    b.seek(begin);

    while (b.tell() < end)
    {
        // OPTIMIZE ?
        buffer_write(out, id, type, value);
    }

    b.seek(pre);
}

void ogm::interpreter::fn::buffer_get_size(VO out, V id)
{
    Buffer& b = frame.m_buffers.get_buffer(id.castCoerce<size_t>());
    out = b.size();
}

void ogm::interpreter::fn::buffer_get_address(VO out, V id)
{
    Buffer& b = frame.m_buffers.get_buffer(id.castCoerce<size_t>());
    out = b.get_address();
    ogm_assert(out.is_pointer());
}

void ogm::interpreter::fn::buffer_save(VO out, V vbuff, V f)
{
    std::string filepath = frame.m_fs.resolve_file_path(f.castCoerce<std::string>(), true);
    std::ofstream output(filepath, std::ios::binary);
    
    Buffer& buff = frame.m_buffers.get_buffer(vbuff.castCoerce<int64_t>());
    
    if (buff.size() > 0)
    {
        output.write(
            static_cast<char*>(buff.get_address()),
            buff.size()
        );
    }
}

void ogm::interpreter::fn::buffer_save_ext(VO out, V vbuff, V f, V voffset, V vsize)
{
    std::string filepath = frame.m_fs.resolve_file_path(f.castCoerce<std::string>(), true);
    std::ofstream output(filepath, std::ios::binary);
    
    Buffer& buff = frame.m_buffers.get_buffer(vbuff.castCoerce<int64_t>());
    int64_t offset = voffset.castCoerce<size_t>();
    int64_t size = vsize.castCoerce<size_t>();
    size = std::min<int64_t>(size, static_cast<int64_t>(buff.size()) - offset);
    
    if (offset >= 0 && size > 0)
    {
        output.write(
            static_cast<char*>(buff.get_address()) + offset,
            size
        );
    }
}

void ogm::interpreter::fn::buffer_load(VO out, V f)
{
    std::string filepath = frame.m_fs.resolve_file_path(f.castCoerce<std::string>(), false);
    std::ifstream input(filepath, std::ios::binary);
    
    if (!input.good())
    {
        out = -1.0;
        return;
    }

    // get size
    std::streampos fsize = 0;
    {
        fsize = input.tellg();
        input.seekg( 0, std::ios::end );
        fsize = input.tellg() - fsize;
        input.seekg( 0, std::ios::beg );
    }

    size_t buffer_index = frame.m_buffers.create_buffer(
        static_cast<size_t>(fsize),
        Buffer::GROW,
        1
    );

    if (fsize > 0)
    {
        input.read(static_cast<char*>(
            frame.m_buffers.get_buffer(buffer_index).get_address()),
            static_cast<size_t>(fsize)
        );
    }

    frame.m_buffers.get_buffer(buffer_index).seek(0);
    out = buffer_index;
}

void ogm::interpreter::fn::game_save_buffer(VO out, V buf)
{
    Buffer& b = frame.m_buffers.get_buffer(buf.castCoerce<size_t>());
    serialize_all<true>(b);
}

void ogm::interpreter::fn::game_load_buffer(VO out, V buf)
{
    Buffer& b = frame.m_buffers.get_buffer(buf.castCoerce<size_t>());
    serialize_all<false>(b);
}

void ogm::interpreter::fn::buffer_copy(VO out, V src, V srcfrom, V len, V dst, V dstto)
{
    // TODO: optimize
    Buffer& bsrc = frame.m_buffers.get_buffer(src.castCoerce<size_t>());
    Buffer& bdst = frame.m_buffers.get_buffer(dst.castCoerce<size_t>());

    size_t l = len.castCoerce<int32_t>();
    if (l == 0) return;

    // set positions
    size_t src_tell = bsrc.tell();
    size_t dst_tell = bdst.tell();
    bsrc.seek(srcfrom.castCoerce<size_t>());
    bdst.seek(dstto.castCoerce<size_t>());

    // copy data
    if (l < 0) throw MiscError("length must be non-negative");
    char* data = new char[l];
    bsrc.read(data, l);
    bdst.write(data, l);

    // restore positions
    bsrc.seek(src_tell);
    bdst.seek(dst_tell);

    // clean up;
    delete[] data;
}
