#include "ogm/project/resource/ResourceSprite.hpp"
#include "ogm/common/util.hpp"
#include "ogm/ast/parse.h"
#include "ogm/project/arf/arf_parse.hpp"
#include "XMLError.hpp"

#include <stb_image.h>
#include <pugixml.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <cstring>

using nlohmann::json;

namespace ogm::project {

ResourceSprite::ResourceSprite(const char* path, const char* name)
    : Resource(name)
    , m_path(path)
{ }

void ResourceSprite::load_file()
{
    if (mark_progress(FILE_LOADED)) return;
    if (ends_with(m_path, ".gmx"))
    {
        load_file_xml();
    }
    else if (ends_with(m_path, ".ogm") || ends_with(m_path, ".arf"))
    {
        load_file_arf();
    }
    else if (ends_with(m_path, ".yy"))
    {
        load_file_json();
    }
    else
    {
        throw ResourceError(1026, this, "Unrecognized file extension for sprite file \"{}\"", m_path);
    }
}

namespace
{
    ARFSchema arf_sprite_schema
    {
        "sprite",
        ARFSchema::DICT,
        {
            "images",
            ARFSchema::LIST
        }
    };
}

void ResourceSprite::load_file_arf()
{
    std::string _path = native_path(m_path);
    std::string _dir = path_directory(_path);
    std::string file_contents = read_file_contents(_path.c_str());

    ARFSection sprite_section;
    
    arf_parse(arf_sprite_schema, file_contents.c_str(), sprite_section);
    
    // this transformation will be applied to the image.
    typedef asset::AssetSprite::SubImage Image;
    std::vector<std::function<Image(Image&)>> transforms;
    std::function<Image(Image&)> transform = [&transforms](Image& in) -> Image
    {
        Image c = in;
        for (auto transform : transforms)
        {
            c = transform(c);
        }
        return c;
    };
    
    // scale
    std::vector<double> scale;
    std::vector<std::string_view> scale_sv;
    std::string scale_s;
    
    scale_s = sprite_section.get_value(
        "scale",
        "[1, 1]"
    );
    
    {
        arf_parse_array(scale_s.c_str(), scale_sv);
        
        for (const std::string_view& sv : scale_sv)
        {
            scale.push_back(svtod_or_percent(std::string(sv)));
        }
        
        // simple scale function.
        if (scale[0] != 1 || scale[1] != 1)
        {
            transforms.emplace_back(
                [s0=scale[0], s1=scale[1]](Image& img) -> Image
                {
                    return img.scaled(s0, s1);
                }
            );
        }
    }
    
    if (scale.size() != 2)
    {
        throw ResourceError(1046, this, "scale must be 2-tuple.");
    }
    
    // xbr
    real_t xbr_amount = std::stoi(sprite_section.get_value(
        "xbr",
        "1"
    ));
    
    if (xbr_amount != 1)
    {
        if (xbr_amount != 2 && xbr_amount != 3 && xbr_amount != 4)
        {
            throw ResourceError(1047, this, "xbr must be 1, 2, 3, or 4.");
        }
        
        bool xbr_alpha = sprite_section.get_value(
            "xbr_scale_alpha",
            "0"
        ) != "0";
        
        bool xbr_blend = sprite_section.get_value(
            "xbr_blend",
            "0"
        ) != "0";
        
        transforms.push_back([xbr_amount, xbr_alpha, xbr_blend](Image& img) -> Image {
            return img.xbr(xbr_amount, xbr_blend, xbr_alpha);
        });
        
        // up scale
        scale[0] *= xbr_amount;
        scale[1] *= xbr_amount;
    }
    
    // sprite images
    {
        // sheet range
        std::vector<int32_t> sheet_range;
        std::vector<std::string_view> sheet_range_sv;
        std::string sheet_range_s;
        
        // negative number = unbounded (i.e. 0 or max).
        sheet_range_s = sprite_section.get_value(
            "sheet_range",
            "[-1, -1]"
        );
        
        arf_parse_array(sheet_range_s.c_str(), sheet_range_sv);
        
        for (const std::string_view& sv : sheet_range_sv)
        {
            sheet_range.push_back(std::stoi(std::string(sv)));
        }
        
        if (sheet_range.size() != 2)
        {
            throw ResourceError(1048, this, "Sheet_range must be 2-tuple. (Hint: negative numbers can be used for unbounded range.)");
        }

        // sheet dimension
        std::vector<int32_t> sheet;
        std::vector<std::string_view> sheet_sv;
        std::string sheet_s;

        sheet_s = sprite_section.get_value(
            "sheet",
            "[1, 1]"
        );
        
        arf_parse_array(sheet_s.c_str(), sheet_sv);
        
        for (const std::string_view& sv : sheet_sv)
        {
            sheet.push_back(std::stoi(std::string(sv)));
        }
        
        if (sheet.size() != 2)
        {
            throw ResourceError(1049, this, "Sheet size must be 2-tuple.");
        }
        
        if (sheet[0] < 1 || sheet[1] < 1)
        {
            throw ResourceError(1050, this, "Sheet dimensions must be positive.");
        }
        
        // clamp negative sheet range
        if (sheet_range[0] < 0) sheet_range[0] = 0;
        if (sheet_range[1] < 0) sheet_range[1] = sheet[0] * sheet[1];

        // subimages
        for (ARFSection* subimages : sprite_section.m_sections)
        {
            for (const std::string& subimage : subimages->m_list)
            {
                Image image{ std::move(_dir + subimage) };
                
                if (sheet[0] > 1 || sheet[1] > 1)
                {
                    // split the image
                    image.realize_data();
                    
                    geometry::Vector<int32_t> subimage_dimensions{
                        image.m_dimensions.x / sheet[0],
                        image.m_dimensions.y / sheet[1],
                    };
                    
                    size_t index = 0;
                    
                    for (int j = 0; j < sheet[1]; ++j)
                    {
                        for (int i = 0; i < sheet[0]; ++i)
                        {
                            if (index >= sheet_range[0] && index < sheet_range[1])
                            {
                                geometry::AABB<int32_t> subimage_region {
                                    subimage_dimensions.x * i,
                                    subimage_dimensions.y * j,
                                    subimage_dimensions.x * (i + 1),
                                    subimage_dimensions.y * (j + 1)
                                };
                                
                                Image image_cropped = image.cropped(subimage_region);
                                
                                if (transforms.size())
                                {
                                    m_subimages.emplace_back(transform(image_cropped));
                                }
                                else
                                {
                                    m_subimages.emplace_back(std::move(image_cropped));
                                }
                            }
                            
                            ++index;
                        }
                    }
                }
                else
                {
                    // just add the image
                    
                    if (transforms.size())
                    {
                        m_subimages.emplace_back(transform(image));
                    }
                    else
                    {
                        m_subimages.emplace_back(std::move(image));
                    }
                }
            }
        }

        if (m_subimages.empty())
        {
            throw ResourceError(1051, this, "needs at least one subimage.");
        }
    }

    std::vector<std::string_view> arr;
    std::string arrs;

    // dimensions
    arrs = sprite_section.get_value(
        "dimensions",
        sprite_section.get_value("size", "[-1, -1]")
    );
    arf_parse_array(arrs.c_str(), arr);
    if (arr.size() != 2) ResourceError(1052, this, "field \"dimensions\" or \"size\" should be a 2-tuple.");
    m_dimensions.x = svtoi(arr[0]) * std::abs(scale[0]);
    m_dimensions.y = svtoi(arr[1]) * std::abs(scale[1]);
    arr.clear();

    if (m_dimensions.x < 0 || m_dimensions.y < 0)
    // load sprite and read its dimensions
    {
        asset::Image& sb = m_subimages.front();

        sb.realize_data();

        m_dimensions = sb.m_dimensions;
    }
    else
    {
        // let subimages know the expected image dimensions (for error-checking later.)
        for (asset::Image& image : m_subimages)
        {
            image.m_dimensions = m_dimensions;
        }
    }

    // origin
    arrs = sprite_section.get_value("origin", "[0, 0]");
    if (sprite_section.has_value("offset")) // alias
    {
        arrs = sprite_section.get_value("offset", "[0, 0]");
    }
    arf_parse_array(arrs.c_str(), arr);
    if (arr.size() != 2) throw ResourceError(1053, this, "field \"origin\" should be a 2-tuple.");
    m_offset.x = scale[0] ? svtod_or_percent(arr[0], m_dimensions.x / scale[0]) * scale[0] : 0;
    m_offset.y = scale[1] ? svtod_or_percent(arr[1], m_dimensions.y / scale[1]) * scale[1] : 0;
    arr.clear();

    // bbox
    if (sprite_section.has_value("bbox"))
    {
        arrs = sprite_section.get_value("bbox", "[0, 0], [1, 1]");
        std::vector<std::string> subarrs;
        split(subarrs, arrs, "x");
        if (subarrs.size() != 2) goto bbox_error;
        for (std::string& s : subarrs)
        {
            trim(s);
        }

        // parse sub-arrays

        // x bounds
        arf_parse_array(std::string{ subarrs.at(0) }.c_str(), arr);
        if (arr.size() != 2) goto bbox_error;
        m_aabb.m_start.x = svtod(arr[0]) * scale[0];
        m_aabb.m_end.x = svtod(arr[1]) * scale[0];
        arr.clear();

        // y bounds
        arf_parse_array(std::string{ subarrs.at(1) }.c_str(), arr);
        if (arr.size() != 2) goto bbox_error;
        m_aabb.m_start.y = svtod(arr[0]) * scale[1];
        m_aabb.m_end.y = svtod(arr[1]) * scale[1];
        arr.clear();

        if (false)
        {
        bbox_error:
            throw ResourceError(1054, this, "field \"bbox\" should be 2-tuple x 2-tuple.");
        }
    }
    else
    {
        m_aabb = { {0, 0}, m_dimensions };
    }
    
    // rotsprite and rotate
    real_t rotsprite_amount = std::stod(sprite_section.get_value(
        "rotsprite",
        "0"
    ));
    
    real_t rotate_amount = std::stod(sprite_section.get_value(
        "rotate",
        "0"
    ));
    
    if ((rotate_amount != 0 || rotsprite_amount != 0) && !m_subimages.empty())
    {
        std::function<geometry::Vector<real_t>(Image&, geometry::Vector<real_t>)> transform;
        if (rotsprite_amount != 0)
        {
            transform = [rotsprite_amount, rotate_amount]
                (Image& img, geometry::Vector<real_t> offset)
                -> geometry::Vector<real_t>
            {
                offset = offset * 8;
                img = img
                    .xbr(2, false, false)
                    .xbr(2, false, false)
                    .xbr(2, false, false)
                    .rotated(rotsprite_amount, offset)
                    .scaled(1/8.0, 1/8.0);
                offset = offset / 8;
                img = img.rotated(rotate_amount, offset);
                return offset;
            };
        }
        else
        {
            transform = [rotate_amount]
                (Image& img, geometry::Vector<real_t> offset)
                -> geometry::Vector<real_t>
            {
                img.realize_data();
                img = img
                    .rotated(rotate_amount, offset);
                return offset;
            };
        }
        
        // center bounds
        m_aabb.m_start -= m_offset;
        m_aabb.m_end -= m_offset;
        
        geometry::Vector<real_t> prev_offset = m_offset;
        for (auto& image : m_subimages)
        {
            m_offset = transform(image, prev_offset);
        }
        
        // rotate bounds
        m_aabb = m_aabb.rotated(rotsprite_amount);
        
        // uncenter bounds
        m_aabb.m_start += m_offset;
        m_aabb.m_end += m_offset;
        
        // update dimensions
        m_dimensions = m_subimages.front().m_dimensions;
    }

    m_separate_collision_masks = svtoi(sprite_section.get_value("sepmasks", "0"));
    m_alpha_tolerance = svtoi(
        sprite_section.get_value("tolerance", "0")
    );

    std::string colkind = sprite_section.get_value(
        "colkind", sprite_section.get_value("collision", "1")
    );
    if (is_digits(colkind))
    {
        m_colkind = std::stoi(colkind);
    }
    else
    {
        if (colkind == "precise" || colkind == "raster")
        {
            m_colkind = 0;
        }
        else if (colkind == "separate")
        {
            m_colkind = 0;
            m_separate_collision_masks = true;
        }
        else if (colkind == "rect" || colkind == "aabb" || colkind == "rectangle" || colkind == "bbox")
        {
            m_colkind = 1;
        }
        else if (colkind == "ellipse" || colkind == "circle")
        {
            m_colkind = 2;
        }
        else if (colkind == "diamond")
        {
            m_colkind = 3;
        }
        else
        {
            throw ResourceError(1055, this, "Unrecognized collision type \'{}\"", colkind);
        }
    }
}

void ResourceSprite::load_file_xml()
{
    const std::string _path = native_path(m_path);
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(_path.c_str(), pugi::parse_default | pugi::parse_escapes);

    check_xml_result<ResourceError>(1064, result, _path.c_str(), "Sprite file not found: " + _path, this);

    pugi::xml_node node = doc.child("sprite");
    pugi::xml_node node_frames = node.child("frames");
    for (pugi::xml_node frame: node_frames.children("frame"))
    {
        std::string frame_index = frame.attribute("index").value();
        size_t index = stoi(frame_index);

        // path to subimage
        bool casechange = false;
        std::string path = case_insensitive_native_path(path_directory(_path), frame.text().get(), &casechange);
        if (!path_exists(path)) throw ResourceError(1056, this, "Failed to find resource at path \"{}\"", path);
        if (casechange) std::cout << "WARNING: path \""<< path_directory(_path) << frame.text().get() << "\" required case-insensitive lookup:\n  Became \"" << path << "\"\n  This is time-consuming and should be corrected.\n";
        if (m_subimages.size() <= index)
        {
            m_subimages.resize(index + 1);
        }
        m_subimages[index].m_path = path;
    }

    const char* node_w = node.child("width").text().get();
    const char* node_h = node.child("height").text().get();
    const char* node_x = node.child("xorig").text().get();
    const char* node_y = node.child("yorigin").text().get();

    const char* bbox_left = node.child("bbox_left").text().get();
    const char* bbox_right = node.child("bbox_right").text().get();
    const char* bbox_top = node.child("bbox_top").text().get();
    const char* bbox_bottom = node.child("bbox_bottom").text().get();

    m_colkind = atoi(node.child("colkind").text().get());
    m_alpha_tolerance = atoi(node.child("coltolerance").text().get());
    m_separate_collision_masks = node.child("sepmasks").text().get() != std::string("0");
    m_bboxmode = atoi(node.child("bboxmode").text().get());

    m_offset = { static_cast<coord_t>(atoi(node_x)), static_cast<coord_t>(atoi(node_y)) };
    m_dimensions = { static_cast<coord_t>(atoi(node_w)), static_cast<coord_t>(atoi(node_h)) };
    m_aabb = {
        { static_cast<coord_t>(atof(bbox_left)), static_cast<coord_t>(atof(bbox_top)) },
        { static_cast<coord_t>(atof(bbox_right)+ 1.0), static_cast<coord_t>(atof(bbox_bottom) + 1.0) }
    };
}

void ResourceSprite::precompile(bytecode::ProjectAccumulator& acc)
{
    const std::string& sprite_name = m_name;
    m_sprite_asset = acc.m_assets->add_asset<asset::AssetSprite>(sprite_name.c_str());

    m_sprite_asset->m_offset = m_offset;
    m_sprite_asset->m_dimensions = m_dimensions;
    m_sprite_asset->m_aabb = m_aabb;

    // we will assign subimage data during compilation proper.

    #ifdef ONLY_RECTANGULAR_COLLISION
    m_sprite_asset->m_shape = asset::AssetSprite::rectangle;
    #else
    switch (m_colkind)
    {
    case 0:
        m_sprite_asset->m_shape = asset::AssetSprite::raster;
        break;
    case 1:
        m_sprite_asset->m_shape = asset::AssetSprite::rectangle;
        break;
    case 2:
        m_sprite_asset->m_shape = asset::AssetSprite::ellipse;
        break;
    case 3:
        m_sprite_asset->m_shape = asset::AssetSprite::diamond;
        break;
    default:
        throw ResourceError(1057, this, "unknown collision shape");
    }
    #endif
}

// adds a raster image to the image data, filled with zeros.
void ResourceSprite::addRaster()
{
    m_sprite_asset->m_raster.emplace_back();
    m_sprite_asset->m_raster.back().m_width = m_sprite_asset->m_dimensions.x;
    const size_t length = m_sprite_asset->m_dimensions.x * m_sprite_asset->m_dimensions.y;
    m_sprite_asset->m_raster.back().m_length = length;
    m_sprite_asset->m_raster.back().m_data = new bool[length];
    for (size_t i = 0; i < length; ++i)
    {
        m_sprite_asset->m_raster.back().m_data[i] = 0;
    }
}

void ResourceSprite::compile(bytecode::ProjectAccumulator&)
{
    if (mark_progress(COMPILED)) return;
    if (m_sprite_asset->m_shape == asset::AssetSprite::raster)
    {
        // precise collision data requires realizing the images.
        if (!m_separate_collision_masks)
        // just add one raster, containing the union of
        // collision data over all frames.
        {
            addRaster();
        }

        for (asset::AssetSprite::SubImage& image : m_subimages)
        {
            if (m_separate_collision_masks)
            // one raster for each frame.
            {
                addRaster();
            }
            collision::CollisionRaster& raster = m_sprite_asset->m_raster.back();
            int channel_count = 4;

            image.realize_data();

            size_t i = 0;
            for (
                uint8_t* c = image.m_data + 3;
                c < image.m_data + image.get_data_len();
                c += channel_count, ++i
            )
            {
                if (*c > m_alpha_tolerance)
                {
                    raster.set(i, true);
                }
            }
        }
    }

    // copy subimage data to sprite.
    m_sprite_asset->m_subimages = m_subimages;
    m_sprite_asset->m_subimage_count = m_subimages.size();
}

void ResourceSprite::load_file_json()
{
    std::fstream ifs(m_path);
    
    if (!ifs.good()) throw ResourceError(1058, this, "Error parsing file \"{}\"", m_path);
    
    json j;
    ifs >> j;
    
    m_v2_id = j.at("id");
    
    for (const json& frame : j.at("frames"))
    {
        asset::AssetSprite::SubImage& subimage = m_subimages.emplace_back();
        
        std::string id = frame.at("id");
        std::string path = path_join(path_directory(m_path), id + ".png");
    }
    
    m_dimensions = {
        j.at("width").get<real_t>(),
        j.at("height").get<real_t>()
    };
    m_offset = {
        j.at("xorig").get<real_t>(),
        j.at("yorig").get<real_t>()
    };
    m_aabb = {
        { j.at("bbox_left").get<real_t>(), j.at("bbox_right").get<real_t>() },
        { j.at("bbox_right").get<real_t>(), j.at("bbox_bottom").get<real_t>() }
    };
    
    m_separate_collision_masks = j.at("sepmasks").get<bool>();
    m_bboxmode = j.at("bboxmode").get<int32_t>();
    m_colkind = j.at("colkind").get<int32_t>();
    m_alpha_tolerance = j.at("coltolerance").get<int32_t>();
}

}
