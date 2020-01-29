#include "library.h"
#include "ogm/interpreter/Variable.hpp"
#include "ogm/common/error.hpp"
#include "ogm/common/util.hpp"
#include "ogm/interpreter/Executor.hpp"
#include "ogm/interpreter/execute.hpp"
#include "ogm/interpreter/display/Display.hpp"

#include <string>
#include <locale>
#include <cctype>
#include <cstdlib>

using namespace ogm::interpreter;
using namespace ogm::interpreter::fn;

#define frame staticExecutor.m_frame
#define display frame.m_display

namespace
{
    // data for the current primitive being added to the model.
    model_id_t g_current_model;
    uint32_t g_current_glenum;
    uint32_t g_current_vao = 0;
}

void ogm::interpreter::fn::d3d_model_create(VO out)
{
    out = display->model_make();
}

void ogm::interpreter::fn::d3d_model_destroy(VO out, V id)
{
    display->model_free(id.castCoerce<model_id_t>());
}

void ogm::interpreter::fn::d3d_model_draw(VO out, V id, V x, V y, V z, V texture)
{
    if (!texture.is_pointer())
    {
        display->model_draw(
            id.castCoerce<model_id_t>(),
            nullptr
        );
    }
    else
    {
        display->model_draw(
            id.castCoerce<model_id_t>(),
            static_cast<TextureView*>(texture.castExact<void*>())->m_tpage
        );
    }
}

void ogm::interpreter::fn::d3d_model_primitive_begin(VO out, V id, V glenum)
{
    Variable dummy;
    g_current_model = id.castCoerce<model_id_t>();
    fn::vertex_create_buffer(dummy);
    g_current_vao = dummy.castCoerce<uint32_t>();
    fn::vertex_begin(out, g_current_vao, dummy);
}

void ogm::interpreter::fn::d3d_model_vertex(VO out, V x, V y, V z)
{
    fn::vertex_position_3d(out, g_current_vao, x, y, z);
    fn::vertex_colour(out, g_current_vao, 0xffffff, 1.0);
    fn::vertex_texcoord(out, g_current_vao, 0, 0);
}


void ogm::interpreter::fn::d3d_model_vertex_texture(VO out, V x, V y, V z, V u, V v)
{
    fn::vertex_position_3d(out, g_current_vao, x, y, z);
    fn::vertex_colour(out, g_current_vao, 0xffffff, 1.0);
    fn::vertex_texcoord(out, g_current_vao, u, v);
}

void ogm::interpreter::fn::d3d_model_vertex_colour(VO out, V x, V y, V z, V colour, V alpha)
{
    fn::vertex_position_3d(out, g_current_vao, x, y, z);
    fn::vertex_colour(out, g_current_vao, colour, alpha);
    fn::vertex_texcoord(out, g_current_vao, 0, 0);
}


void ogm::interpreter::fn::d3d_model_vertex_texture_colour(VO out, V x, V y, V z, V u, V v, V colour, V alpha)
{
    fn::vertex_position_3d(out, g_current_vao, x, y, z);
    fn::vertex_colour(out, g_current_vao, colour, alpha);
    fn::vertex_texcoord(out, g_current_vao, u, v);
}

void ogm::interpreter::fn::d3d_model_primitive_end(VO out)
{
    fn::vertex_end(out, g_current_vao);
    fn::vertex_freeze(out, g_current_vao);
    g_current_vao = 0;
}