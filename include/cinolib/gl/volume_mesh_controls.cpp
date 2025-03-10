/********************************************************************************
*  This file is part of CinoLib                                                 *
*  Copyright(C) 2021: Marco Livesu                                              *
*                                                                               *
*  The MIT License                                                              *
*                                                                               *
*  Permission is hereby granted, free of charge, to any person obtaining a      *
*  copy of this software and associated documentation files (the "Software"),   *
*  to deal in the Software without restriction, including without limitation    *
*  the rights to use, copy, modify, merge, publish, distribute, sublicense,     *
*  and/or sell copies of the Software, and to permit persons to whom the        *
*  Software is furnished to do so, subject to the following conditions:         *
*                                                                               *
*  The above copyright notice and this permission notice shall be included in   *
*  all copies or substantial portions of the Software.                          *
*                                                                               *
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR   *
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,     *
*  FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT. IN NO EVENT SHALL THE *
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER       *
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING      *
*  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS *
*  IN THE SOFTWARE.                                                             *
*                                                                               *
*  Author(s):                                                                   *
*                                                                               *
*     Marco Livesu (marco.livesu@gmail.com)                                     *
*     http://pers.ge.imati.cnr.it/livesu/                                       *
*                                                                               *
*     Italian National Research Council (CNR)                                   *
*     Institute for Applied Mathematics and Information Technologies (IMATI)    *
*     Via de Marini, 6                                                          *
*     16149 Genoa,                                                              *
*     Italy                                                                     *
*********************************************************************************/
#include <cinolib/gl/volume_mesh_controls.h>
#include <../external/imgui/imgui.h>
#include <cinolib/gl/file_dialog_open.h>
#include <cinolib/gl/file_dialog_save.h>
#include <cinolib/gradient.h>
#include <cinolib/scalar_field.h>
#include <cinolib/string_utilities.h>
#include <cinolib/export_visible.h>
#include <cinolib/export_surface.h>
#include <cinolib/export_marked_faces.h>
#include <cinolib/ambient_occlusion.h>
#include <iostream>
#include <sstream>

namespace cinolib
{

template <class Mesh>
CINO_INLINE
VolumeMeshControls<Mesh>::VolumeMeshControls(Mesh *m, GLcanvas *gui, const std::string & _name)
    : SideBarItem(_name)
    , m(m)
    , gui(gui)
{
    // if no name is provided, assign name memorized in the mesh data (typically the filename)
    if(_name.empty()) this->name = get_file_name(m->mesh_data().filename);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::draw()
{
    if(m==nullptr || gui==nullptr) return;

    header_IO                 (false);
    header_shading            (false);
    header_wireframe          (false);
    header_colors_textures_in (false);
    header_colors_textures_out(false);
    header_vector_field       (false);
    header_isosurface         (false);
    header_slicing            (false);
    header_marked_edges       (false);
    header_marked_faces       (false);
    header_ambient_occlusion  (false);
    header_manual_digging     (false);
    header_actions            (false);
    header_debug              (false);
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::header_IO(const bool open)
{
    ImGui::SetNextItemOpen(open, ImGuiCond_Once);
    if(ImGui::TreeNode("IO"))
    {
        if(ImGui::SmallButton("Load"))
        {
            std::string filename = file_dialog_open();
            if(!filename.empty())
            {
                *m = Mesh(filename.c_str());
                gui->refit_scene();
            }
        }
        ImGui::SameLine();
        if(ImGui::SmallButton("Save"))
        {
            std::string filename = file_dialog_save();
            if(!filename.empty())
            {
                m->save(filename.c_str());
            }
        }
        ImGui::TreePop();
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::header_shading(const bool open)
{
    ImGui::SetNextItemOpen(open,ImGuiCond_Once);
    if(ImGui::TreeNode("Shading"))
    {
        if(ImGui::Checkbox("Show##mesh", &show_mesh)) m->show_mesh(show_mesh);
        if(ImGui::RadioButton("Point ",  m->drawlist_in.draw_mode & DRAW_TRI_POINTS)) m->show_mesh_points();
        if(ImGui::RadioButton("Flat  ",  m->drawlist_in.draw_mode & DRAW_TRI_FLAT  )) m->show_mesh_flat();
        if(ImGui::RadioButton("Smooth",  m->drawlist_in.draw_mode & DRAW_TRI_SMOOTH)) m->show_mesh_smooth();
        ImGui::TreePop();
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::header_wireframe(const bool open)
{
    ImGui::SetNextItemOpen(open,ImGuiCond_Once);
    if(ImGui::TreeNode("Wireframe"))
    {
        if(ImGui::Checkbox("Show", &show_wireframe))
        {
            m->show_in_wireframe(show_wireframe);
            m->show_out_wireframe(show_wireframe);
        }
        if(ImGui::SliderInt("Width", &wireframe_width, 1, 10))
        {
            m->show_in_wireframe_width(float(wireframe_width));
            m->show_out_wireframe_width(float(wireframe_width));
        }
        if(ImGui::SliderFloat("Transparency", &wireframe_alpha, 0.f, 1.f))
        {
            m->show_in_wireframe_transparency(wireframe_alpha);
            m->show_out_wireframe_transparency(wireframe_alpha);
        }
        ImGui::TreePop();
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::header_colors_textures_in(const bool open)
{
    ImGui::SetNextItemOpen(open,ImGuiCond_Once);
    if(ImGui::TreeNode("Colors/Textures (in)"))
    {
        if(ImGui::BeginTable("Color by:",2))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if(ImGui::RadioButton("Vert", m->drawlist_in.draw_mode & DRAW_TRI_VERTCOLOR))
            {
                m->show_in_vert_color();
            }
            ImGui::TableNextColumn();
            if(ImGui::ColorEdit4("##VertColor", vert_color.rgba, color_edit_flags))
            {
                m->vert_set_color(vert_color);
                m->show_in_vert_color();
            }
            //
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if(ImGui::RadioButton("Poly", m->drawlist_in.draw_mode & DRAW_TRI_FACECOLOR))
            {
                m->show_in_poly_color();
            }
            ImGui::TableNextColumn();
            if(ImGui::ColorEdit4("##PolyColor", poly_color.rgba, color_edit_flags))
            {
                m->poly_set_color(poly_color);
                m->show_in_poly_color();
            }
            //
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if(ImGui::RadioButton("Tex1D", m->drawlist_in.draw_mode & DRAW_TRI_TEXTURE1D))
            {
                m->show_in_texture1D(m->drawlist_in.texture.type);
            }
            ImGui::TableNextColumn();
            if(ImGui::Combo("##1DTexture", &m->drawlist_in.texture.type, "isolines\0HSV\0HSV+isolines\0Parula\0Parula+isolines\0"))
            {
                m->show_in_texture1D(m->drawlist_in.texture.type);
            }
            //
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if(ImGui::RadioButton("Tex2D", m->drawlist_in.draw_mode & DRAW_TRI_TEXTURE2D))
            {
                // m->drawlist_in.texture.type indexes both 1D and 2D textures, but there are more 1D than 2D!
                if(m->drawlist_in.texture.type<2) m->show_in_texture2D(m->drawlist_in.texture.type, m->drawlist_in.texture.scaling_factor);
            }
            ImGui::TableNextColumn();
            if(ImGui::Combo("##2DTexture", &m->drawlist_in.texture.type, "isolines\0Checkerboard\0File\0"))
            {
                switch(m->drawlist_in.texture.type)
                {
                    case TEXTURE_2D_ISOLINES:
                    case TEXTURE_2D_CHECKERBOARD: m->show_in_texture2D(m->drawlist_in.texture.type, m->drawlist_in.texture.scaling_factor); break;
                    case TEXTURE_2D_BITMAP:
                    {
                        std::string filename = file_dialog_open();
                        if(!filename.empty())
                        {
                            m->show_in_texture2D(TEXTURE_2D_BITMAP, m->drawlist_in.texture.scaling_factor, filename.c_str());
                        }
                        break;
                    }
                    default : break; // texture.type indexes both 1D and 2D textures, but there are more 1D indices than 2D!
                }
            }
            ImGui::EndTable();
            //
            if(ImGui::SliderFloat("Tex scaling", &m->drawlist_in.texture.scaling_factor, 0.01f, 100.f))
            {
                if(m->drawlist_in.texture.type!=TEXTURE_2D_BITMAP)
                {
                    m->show_in_texture2D(m->drawlist_in.texture.type,m->drawlist_in.texture.scaling_factor);
                }
            }
        }
        ImGui::TreePop();
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::header_colors_textures_out(const bool open)
{
    ImGui::SetNextItemOpen(open,ImGuiCond_Once);
    if(ImGui::TreeNode("Colors/Textures (out)"))
    {
        if(ImGui::BeginTable("Color by:",2))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if(ImGui::RadioButton("Vert", m->drawlist_out.draw_mode & DRAW_TRI_VERTCOLOR))
            {
                m->show_out_vert_color();
            }
            ImGui::TableNextColumn();
            if(ImGui::ColorEdit4("##VertColor", vert_color.rgba, color_edit_flags))
            {
                m->vert_set_color(vert_color);
                m->show_out_vert_color();
            }
            //
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if(ImGui::RadioButton("Poly", m->drawlist_out.draw_mode & DRAW_TRI_FACECOLOR))
            {
                m->show_out_poly_color();
            }
            ImGui::TableNextColumn();
            if(ImGui::ColorEdit4("##PolyColor", poly_color.rgba, color_edit_flags))
            {
                m->poly_set_color(poly_color);
                m->show_out_poly_color();
            }
            //
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if(ImGui::RadioButton("Tex1D", m->drawlist_out.draw_mode & DRAW_TRI_TEXTURE1D))
            {
                m->show_out_texture1D(m->drawlist_out.texture.type);
            }
            ImGui::TableNextColumn();
            if(ImGui::Combo("##1DTexture", &m->drawlist_out.texture.type, "isolines\0HSV\0HSV+isoline\0Parula\0Parula+isolines\0"))
            {
                m->show_out_texture1D(m->drawlist_out.texture.type);
            }
            //
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if(ImGui::RadioButton("Tex2D", m->drawlist_out.draw_mode & DRAW_TRI_TEXTURE2D))
            {
                // m->drawlist_out.texture.type indexes both 1D and 2D textures, but there are more 1D than 2D!
                if(m->drawlist_out.texture.type<2) m->show_out_texture2D(m->drawlist_out.texture.type, m->drawlist_out.texture.scaling_factor);
            }
            ImGui::TableNextColumn();
            if(ImGui::Combo("##2DTexture", &m->drawlist_out.texture.type, "isolines\0Checkerboard\0File\0"))
            {
                switch(m->drawlist_out.texture.type)
                {
                    case TEXTURE_2D_ISOLINES:
                    case TEXTURE_2D_CHECKERBOARD: m->show_out_texture2D(m->drawlist_out.texture.type, m->drawlist_out.texture.scaling_factor); break;
                    case TEXTURE_2D_BITMAP:
                    {
                        std::string filename = file_dialog_open();
                        if(!filename.empty())
                        {
                            m->show_out_texture2D(TEXTURE_2D_BITMAP, m->drawlist_out.texture.scaling_factor, filename.c_str());
                        }
                        break;
                    }
                    default : break; // texture.type indexes both 1D and 2D textures, but there are more 1D indices than 2D!
                }
            }
            ImGui::EndTable();
            //
            if(ImGui::SliderFloat("Tex scaling", &m->drawlist_out.texture.scaling_factor, 0.01f, 100.f))
            {
                if(m->drawlist_out.texture.type!=TEXTURE_2D_BITMAP)
                {
                    m->show_out_texture2D(m->drawlist_out.texture.type,m->drawlist_out.texture.scaling_factor);
                }
            }
        }
        ImGui::TreePop();
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template<class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::header_isosurface(const bool open)
{
    ImGui::SetNextItemOpen(open,ImGuiCond_Once);
    if(ImGui::TreeNode("Isosurface"))
    {
        auto update_isosurface = [&]()
        {
            Tetmesh<M,V,E,F,P> *ptr = dynamic_cast<Tetmesh<M,V,E,F,P>*>(m);
            isosurface              = DrawableIsosurface<M,V,E,F,P>(*ptr, iso_val);
            isosurface.color        = iso_color;
            gui->push(&isosurface,false);
        };
        if(ImGui::Checkbox("Show##isosurface", &show_isosurface))
        {
            if(show_isosurface) update_isosurface();
            else                gui->pop(&isosurface);
        }
        if(ImGui::SliderFloat("Val", &iso_val,iso_min,iso_max))
        {
            if(show_isosurface)
            {
                gui->pop(&isosurface);
                update_isosurface();
            }
        }
        if(ImGui::SmallButton("Update Range"))
        {
            iso_max = float(m->vert_max_uvw_value(U_param));
            iso_min = float(m->vert_min_uvw_value(U_param));
            iso_val = (iso_max-iso_min)*0.5f;
        }
        if(ImGui::SmallButton("Export"))
        {
            std::string filename = file_dialog_save();
            if(!filename.empty()) isosurface.export_as_trimesh().save(filename.c_str());
        }
        if(ImGui::ColorEdit4("Color##isosurface", iso_color.rgba, color_edit_flags))
        {
            if(show_isosurface)
            {
                gui->pop(&isosurface);
                update_isosurface();
            }
        }
        if(ImGui::SmallButton("Tessellate"))
        {
            Tetmesh<M,V,E,F,P> *ptr = dynamic_cast<Tetmesh<M,V,E,F,P>*>(m);
            isosurface.tessellate(*ptr);
            m->updateGL();
        }
        if(ImGui::SmallButton("Load 1D field"))
        {
            std::string filename = file_dialog_open();
            if(!filename.empty())
            {
                ScalarField sf(filename.c_str());
                if ((uint)sf.size() == m->num_verts()) sf.copy_to_mesh(*m);
                else std::cerr << "Could not load scalar field " << filename << " - array size mismatch!" << std::endl;
            }
        }
        if(ImGui::SmallButton("Save 1D field"))
        {
            std::string filename = file_dialog_save();
            if(!filename.empty())
            {
                ScalarField sf(m->serialize_uvw(U_param));
                sf.serialize(filename.c_str());
            }
        }
        ImGui::TreePop();
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::header_vector_field(const bool open)
{
    ImGui::SetNextItemOpen(open,ImGuiCond_Once);
    if(ImGui::TreeNode("Vector Fields"))
    {
        if(ImGui::Checkbox("Show##vecfield", &show_vecfield))
        {
            if(show_vecfield)
            {
                if(vec_field.size()==0) // if the field is empty compute the gradient
                {
                    vec_field = DrawableVectorField(*m);
                    ScalarField f(m->serialize_uvw(U_param));
                    vec_field = gradient_matrix(*m) * f;
                    vec_field.normalize();
                    vec_field.set_arrow_size(float(m->edge_avg_length())*vecfield_size);
                    vec_field.set_arrow_color(vec_color);
                }
                gui->push(&vec_field,false);
            }
            else gui->pop(&vec_field);
        }
        ImGui::SameLine();
        if(ImGui::SmallButton("Load##vecfield"))
        {
            std::string filename = file_dialog_open();
            if(!filename.empty())
            {
                vec_field = DrawableVectorField(*m);
                vec_field.deserialize(filename.c_str());
                vec_field.set_arrow_size(float(m->edge_avg_length())*vecfield_size);
                vec_field.set_arrow_color(vec_color);
            }
        }
        ImGui::SameLine();
        if(ImGui::SmallButton("Save##vecfield"))
        {
            std::string filename = file_dialog_save();
            if(!filename.empty()) vec_field.serialize(filename.c_str());
        }
        if(ImGui::SliderFloat("Size", &vecfield_size, 0.1f, 5.f))
        {
            vec_field.set_arrow_size(float(m->edge_avg_length())*vecfield_size);
        }
        if(ImGui::ColorEdit4("Color##vec", vec_color.rgba, color_edit_flags))
        {
            vec_field.set_arrow_color(vec_color);
        }
        ImGui::TreePop();
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::header_slicing(const bool open)
{
    ImGui::SetNextItemOpen(open,ImGuiCond_Once);
    if(ImGui::TreeNode("Slicing"))
    {
        bool refresh = false;
        if(ImGui::SmallButton("Copy"))
        {
            glfwSetClipboardString(gui->window,slicer.serialize().c_str());
        }
        ImGui::SameLine();
        if(ImGui::SmallButton("Paste"))
        {
            slicer.deserialize(glfwGetClipboardString(gui->window));
            refresh = true;
        }
        ImGui::SameLine();
        if(ImGui::SmallButton("Reset"))
        {
            slicer.reset();
            refresh = true;
        }
        refresh |= ImGui::RadioButton("AND", (int*)&slicer.mode_AND, 1); ImGui::SameLine();
        refresh |= ImGui::RadioButton("OR ", (int*)&slicer.mode_AND, 0);
        refresh |= ImGui::SliderFloat("X",   &slicer.X_thresh, 0.f, 1.f); ImGui::SameLine();
        refresh |= ImGui::Checkbox   ("##x", &slicer.X_leq);
        refresh |= ImGui::SliderFloat("Y",   &slicer.Y_thresh, 0.f, 1.f); ImGui::SameLine();
        refresh |= ImGui::Checkbox   ("##y", &slicer.Y_leq);
        refresh |= ImGui::SliderFloat("Z",   &slicer.Z_thresh, 0.f, 1.f); ImGui::SameLine();
        refresh |= ImGui::Checkbox   ("##z", &slicer.Z_leq);
        refresh |= ImGui::SliderFloat("Q",   &slicer.Q_thresh, 0.f, 1.f); ImGui::SameLine();
        refresh |= ImGui::Checkbox   ("##q", &slicer.Q_leq);
        refresh |= ImGui::SliderInt  ("L",   &slicer.L_filter, -1, 10); ImGui::SameLine();
        refresh |= ImGui::Checkbox   ("##l", &slicer.L_is);
        if(refresh)
        {
            slicer.slice(*m);
            m->updateGL();
        }
        ImGui::TreePop();
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::header_marked_edges(const bool open)
{
    ImGui::SetNextItemOpen(open,ImGuiCond_Once);
    if(ImGui::TreeNode("Marked Edges"))
    {
        if(ImGui::Checkbox("Show##MarkedEdge", &show_marked_edges))
        {
            m->show_marked_edge(show_marked_edges);
        }
        if(ImGui::SliderInt("Width##2", &marked_edge_width, 1, 10))
        {
            m->show_marked_edge_width(float(marked_edge_width));
            m->updateGL();
        }
        if(ImGui::ColorEdit4("Color##markededge", marked_edge_color.rgba, color_edit_flags))
        {
            m->show_marked_edge_color(marked_edge_color);
            m->updateGL();
        }
        ImGui::TreePop();
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::header_marked_faces(const bool open)
{
    ImGui::SetNextItemOpen(open,ImGuiCond_Once);
    if(ImGui::TreeNode("Marked Faces"))
    {
        if(ImGui::Checkbox("Show##MarkedFace", &show_marked_faces))
        {
            m->show_marked_face(show_marked_faces);
        }
        if(ImGui::ColorEdit4("Color##markedface", marked_face_color.rgba, color_edit_flags))
        {
            m->show_marked_face_color(marked_face_color);
            m->updateGL();
        }
        ImGui::TreePop();
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::header_ambient_occlusion(const bool open)
{
    ImGui::SetNextItemOpen(open,ImGuiCond_Once);
    if(ImGui::TreeNode("AO"))
    {
        if(ImGui::SmallButton("Update AO"))
        {
            ambient_occlusion_vol_meshes(*m);
            m->updateGL();
        }
        if(ImGui::SliderFloat("Alpha",&m->AO_alpha, 0.f, 1.f))
        {
            m->updateGL();
        }
        ImGui::TreePop();
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::header_manual_digging(const bool open)
{
    //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    static auto func_dig = [&](int modifiers) -> bool
    {
        if(modifiers & GLFW_MOD_SHIFT)
        {
            vec3d p;
            vec2d click = gui->cursor_pos();
            if(gui->unproject(click, p)) // transform click in a 3d point
            {
                uint fid = m->pick_face(p);
                for(uint pid : m->adj_f2p(fid))
                {
                    if(m->poly_data(pid).flags[HIDDEN] == false)
                    {
                        m->poly_data(pid).flags[HIDDEN] = true;
                    }
                }
                m->updateGL();
            }
        }
        return false;
    };

    //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    static auto func_undig = [&](int modifiers) -> bool
    {
        if(modifiers & GLFW_MOD_SHIFT)
        {
            vec3d p;
            vec2d click = gui->cursor_pos();
            if(gui->unproject(click, p)) // transform click in a 3d point
            {
                uint fid = m->pick_face(p);
                for(uint pid : m->adj_f2p(fid))
                {
                    if(m->poly_data(pid).flags[HIDDEN] == true)
                    {
                        m->poly_data(pid).flags[HIDDEN] = false;
                    }
                }
                m->updateGL();
            }
        }
        return false;
    };

    //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    static auto func_isolate = [&](int modifiers) -> bool
    {
        if(modifiers & GLFW_MOD_SHIFT)
        {
            vec3d p;
            vec2d click = gui->cursor_pos();
            if(gui->unproject(click, p)) // transform click in a 3d point
            {
                uint fid = m->pick_face(p);
                uint pid_beneath;
                if(!m->face_is_visible(fid,pid_beneath))
                {
                    // not a good selection...
                    return false;
                }
                for(uint vid : m->adj_p2v(pid_beneath))
                for(uint pid : m->adj_v2p(vid))
                {
                    if(m->poly_data(pid).flags[HIDDEN] == false)
                    {
                        m->poly_data(pid).flags[HIDDEN] = true;
                    }
                }
                m->poly_data(pid_beneath).flags[HIDDEN] = false;
                m->updateGL();
            }
        }
        return false;
    };

    //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    ImGui::SetNextItemOpen(open,ImGuiCond_Once);
    if(ImGui::TreeNode("Manual Digging"))
    {
        if(ImGui::RadioButton("Dig    ", &dig_choice, DIG    )) gui->callback_mouse_left_click = func_dig;
        if(ImGui::RadioButton("Undig  ", &dig_choice, UNDIG  )) gui->callback_mouse_left_click = func_undig;
        if(ImGui::RadioButton("Isolate", &dig_choice, ISOLATE)) gui->callback_mouse_left_click = func_isolate;
        if(ImGui::RadioButton("Reset  ", &dig_choice, RESET  )) gui->callback_mouse_left_click = nullptr;
        ImGui::TreePop();
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::header_debug(const bool open)
{
    ImGui::SetNextItemOpen(open,ImGuiCond_Once);
    if(ImGui::TreeNode("Debug"))
    {
        if(ImGui::Checkbox("Show Vert Normals", &show_vert_normals))
        {
            if(show_vert_normals)
            {
                vert_normals.clear();
                vert_normals.use_gl_lines  = true;
                vert_normals.default_color = vert_debug_color;
                double l = gui->camera.scene_radius/5.0;
                for(uint vid=0; vid<m->num_verts(); ++vid)
                {
                    if(m->vert_is_visible(vid))
                    {
                        vec3d n = m->vert_data(vid).normal;
                        vec3d p = m->vert(vid);
                        vert_normals.push_seg(p, p+(n*l));
                    }
                }
                gui->push(&vert_normals,false);
            }
            else
            {
                gui->pop(&vert_normals);
            }
        }
        if(ImGui::Checkbox("Show Face Normals", &show_face_normals))
        {
            if(show_face_normals)
            {
                face_normals.clear();
                face_normals.use_gl_lines  = true;
                face_normals.default_color = face_debug_color;
                double l = gui->camera.scene_radius/5.0;
                for(uint fid=0; fid<m->num_faces(); ++fid)
                {
                    uint pid_beneath;
                    if(m->face_is_visible(fid, pid_beneath))
                    {
                        vec3d  n = m->poly_face_normal(pid_beneath,fid);
                        vec3d  c = m->face_centroid(fid);
                        face_normals.push_seg(c, c+(n*l));
                    }
                }
                gui->push(&face_normals,false);
            }
            else
            {
                gui->pop(&face_normals);
            }
        }
        if(ImGui::Checkbox("Show Vert IDs", &show_vert_ids)                           ||
           ImGui::Checkbox("Show Face IDs", &show_face_ids)                           ||
           ImGui::Checkbox("Depth Cull IDs", &gui->depth_cull_markers)                ||
           ImGui::SliderInt("Font", &marker_font_size, 1, 15)                         ||
           ImGui::SliderInt("Disk", &marker_size, 0,10)                               ||
           ImGui::ColorEdit4("Vert Color", vert_debug_color.rgba, color_edit_flags) ||
           ImGui::ColorEdit4("Face Color", face_debug_color.rgba, color_edit_flags))
        {
            gui->pop_all_markers();
            if(show_vert_ids)
            {
                for(uint vid=0; vid<m->num_verts(); ++vid)
                {
                    if(m->vert_is_visible(vid))
                    {
                        gui->push_marker(m->vert(vid), std::to_string(vid), vert_debug_color, marker_size, marker_font_size);
                    }
                }
            }
            if(show_face_ids)
            {
                for(uint fid=0; fid<m->num_faces(); ++fid)
                {
                    uint pid_beneath;
                    if(m->face_is_visible(fid, pid_beneath))
                    {
                        gui->push_marker(m->face_centroid(fid), std::to_string(fid), face_debug_color, marker_size, marker_font_size);
                    }
                }
            }
        }
        ImGui::TreePop();
    }
}

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

template <class Mesh>
CINO_INLINE
void VolumeMeshControls<Mesh>::header_actions(const bool open)
{
    ImGui::SetNextItemOpen(open,ImGuiCond_Once);
    if(ImGui::TreeNode("Actions"))
    {
        bool refresh = false;
        if(ImGui::SmallButton("Unmark all edges"))
        {
            m->edge_set_flag(MARKED,false);
            refresh = true;
        }
        if(ImGui::SmallButton("Unmark all faces"))
        {
            m->face_set_flag(MARKED,false);
            refresh = true;
        }
        if(ImGui::SmallButton("Color wrt Label"))
        {
            m->poly_color_wrt_label();
            refresh = true;
        }
        if(ImGui::SmallButton("Label wrt Color"))
        {
            m->poly_label_wrt_color();
            refresh = true;
        }
        if(ImGui::SmallButton("Mark color discontinuities"))
        {
            for(uint fid=0; fid<m->num_faces(); ++fid)
            {
                if(m->face_is_on_srf(fid))continue;
                uint p0 = m->adj_f2p(fid).front();
                uint p1 = m->adj_f2p(fid).back();
                m->face_data(fid).flags[MARKED] = (m->poly_data(p0).label!=m->poly_data(p1).label);
            }
            for(uint eid=0; eid<m->num_edges(); ++eid)
            {
                uint count = 0;
                for(uint fid : m->adj_e2f(eid))
                {
                    if(m->face_data(fid).flags[MARKED]) ++count;
                }
                m->edge_data(eid).flags[MARKED] = (m->edge_is_on_srf(eid) && count>=2) || (count>2);
            }
            refresh = true;
        }
        if(ImGui::SmallButton("Export Visible"))
        {
            Polyhedralmesh<M,V,E,F,P> tmp;
            export_visible(*m, tmp);
            std::string filename = file_dialog_save();
            if(!filename.empty()) tmp.save(filename.c_str());
            refresh = true;
        }
        if(ImGui::SmallButton("Export Marked Faces"))
        {
            Polygonmesh<M,V,E,F> tmp;
            export_marked_faces(*m, tmp);
            std::string filename = file_dialog_save();
            if(!filename.empty()) tmp.save(filename.c_str());
            refresh = true;
        }
        if(ImGui::SmallButton("Export Surface"))
        {
            Polygonmesh<M,V,E,F> tmp;
            export_surface(*m, tmp);
            std::string filename = file_dialog_save();
            if(!filename.empty()) tmp.save(filename.c_str());
            refresh = true;
        }
        if(ImGui::SmallButton("Mark Creases"))
        {
            m->edge_mark_sharp_creases(float(to_rad(crease_deg)));
            refresh = true;
        }
        ImGui::InputInt("deg", &crease_deg);

        if(refresh) m->updateGL();
        ImGui::TreePop();
    }
}

}
