#include "render/GltfModel.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "stb_image.h"

namespace {

static std::string GetDir(const char* path) {
    if (!path) {
        return std::string();
    }
    std::string p(path);
    size_t slash = p.find_last_of("/\\");
    if (slash == std::string::npos) {
        return std::string();
    }
    return p.substr(0, slash + 1);
}

static bool StartsWith(const char* s, const char* prefix) {
    if (!s || !prefix) {
        return false;
    }
    size_t len = std::strlen(prefix);
    return std::strncmp(s, prefix, len) == 0;
}

static size_t Base64DecodedSize(const char* base64) {
    size_t len = std::strlen(base64);
    size_t padding = 0;
    if (len >= 1 && base64[len - 1] == '=') {
        padding += 1;
    }
    if (len >= 2 && base64[len - 2] == '=') {
        padding += 1;
    }
    return (len / 4) * 3 - padding;
}

static GLuint CreateTextureFromRGBA(const unsigned char* data, int width, int height) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

static GLuint LoadImageTexture(const cgltf_image* image, const cgltf_options& options, const std::string& base_dir) {
    if (!image) {
        return 0;
    }

    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    int comp = 0;

    stbi_set_flip_vertically_on_load(1);

    if (image->buffer_view && image->buffer_view->buffer && image->buffer_view->buffer->data) {
        const unsigned char* data = static_cast<const unsigned char*>(image->buffer_view->buffer->data);
        const unsigned char* bytes = data + image->buffer_view->offset;
        size_t size = image->buffer_view->size;
        pixels = stbi_load_from_memory(bytes, static_cast<int>(size), &width, &height, &comp, 4);
    } else if (image->uri && StartsWith(image->uri, "data:")) {
        const char* comma = std::strchr(image->uri, ',');
        if (comma && std::strstr(image->uri, "base64")) {
            const char* base64 = comma + 1;
            size_t decoded_size = Base64DecodedSize(base64);
            void* decoded = nullptr;
            if (cgltf_load_buffer_base64(&options, decoded_size, base64, &decoded) == cgltf_result_success) {
                pixels = stbi_load_from_memory(static_cast<unsigned char*>(decoded),
                                               static_cast<int>(decoded_size),
                                               &width, &height, &comp, 4);
                options.memory.free_func ? options.memory.free_func(options.memory.user_data, decoded)
                                         : free(decoded);
            }
        }
    } else if (image->uri) {
        std::string full_path = base_dir + image->uri;
        pixels = stbi_load(full_path.c_str(), &width, &height, &comp, 4);
    }

    if (!pixels) {
        return 0;
    }

    GLuint tex = CreateTextureFromRGBA(pixels, width, height);
    stbi_image_free(pixels);
    return tex;
}

static int TextureIndexFromMaterial(const cgltf_material* material, const cgltf_data* data) {
    if (!material) {
        return -1;
    }
    const cgltf_texture* texture = material->pbr_metallic_roughness.base_color_texture.texture;
    if (!texture || !texture->image) {
        return -1;
    }
    return static_cast<int>(texture->image - data->images);
}

static void CopyBaseColor(const cgltf_material* material, float out_color[4]) {
    if (!material) {
        out_color[0] = 1.0f;
        out_color[1] = 1.0f;
        out_color[2] = 1.0f;
        out_color[3] = 1.0f;
        return;
    }
    const float* factor = material->pbr_metallic_roughness.base_color_factor;
    out_color[0] = factor[0];
    out_color[1] = factor[1];
    out_color[2] = factor[2];
    out_color[3] = factor[3];
}

}  // namespace

bool LoadGltfModel(const char* path, GltfModel& out_model) {
    out_model.primitives.clear();
    out_model.textures.clear();

    cgltf_options options{};
    cgltf_data* data = nullptr;
    cgltf_result result = cgltf_parse_file(&options, path, &data);
    if (result != cgltf_result_success || !data) {
        std::printf("Failed to parse glTF: %s\n", path ? path : "(null)");
        return false;
    }

    result = cgltf_load_buffers(&options, data, path);
    if (result != cgltf_result_success) {
        std::printf("Failed to load glTF buffers: %s\n", path ? path : "(null)");
        cgltf_free(data);
        return false;
    }

    std::string base_dir = GetDir(path);

    out_model.textures.resize(data->images_count);
    for (cgltf_size i = 0; i < data->images_count; ++i) {
        out_model.textures[i] = LoadImageTexture(&data->images[i], options, base_dir);
    }

    for (cgltf_size n = 0; n < data->nodes_count; ++n) {
        const cgltf_node* node = &data->nodes[n];
        if (!node->mesh) {
            continue;
        }

        float node_matrix[16];
        cgltf_node_transform_world(node, node_matrix);

        const cgltf_mesh* mesh = node->mesh;
        for (cgltf_size p = 0; p < mesh->primitives_count; ++p) {
            const cgltf_primitive* prim = &mesh->primitives[p];
            if (prim->type != cgltf_primitive_type_triangles) {
                continue;
            }

            const cgltf_accessor* pos_acc = nullptr;
            const cgltf_accessor* uv_acc = nullptr;

            for (cgltf_size a = 0; a < prim->attributes_count; ++a) {
                const cgltf_attribute* attr = &prim->attributes[a];
                if (attr->type == cgltf_attribute_type_position) {
                    pos_acc = attr->data;
                } else if (attr->type == cgltf_attribute_type_texcoord && attr->index == 0) {
                    uv_acc = attr->data;
                }
            }

            if (!pos_acc) {
                continue;
            }

            size_t vertex_count = pos_acc->count;
            std::vector<float> vertices(vertex_count * 5);
            for (size_t i = 0; i < vertex_count; ++i) {
                float pos[3] = {0.0f, 0.0f, 0.0f};
                cgltf_accessor_read_float(pos_acc, i, pos, 3);
                float uv[2] = {0.0f, 0.0f};
                if (uv_acc) {
                    cgltf_accessor_read_float(uv_acc, i, uv, 2);
                }
                size_t base = i * 5;
                vertices[base + 0] = pos[0];
                vertices[base + 1] = pos[1];
                vertices[base + 2] = pos[2];
                vertices[base + 3] = uv[0];
                vertices[base + 4] = uv[1];
            }

            std::vector<unsigned int> indices;
            if (prim->indices) {
                size_t index_count = prim->indices->count;
                indices.resize(index_count);
                for (size_t i = 0; i < index_count; ++i) {
                    indices[i] = static_cast<unsigned int>(cgltf_accessor_read_index(prim->indices, i));
                }
            } else {
                indices.resize(vertex_count);
                for (size_t i = 0; i < vertex_count; ++i) {
                    indices[i] = static_cast<unsigned int>(i);
                }
            }

            GltfPrimitive out_prim{};
            out_prim.index_count = static_cast<int>(indices.size());
            out_prim.has_uv = uv_acc != nullptr;
            std::memcpy(out_prim.transform, node_matrix, sizeof(node_matrix));
            CopyBaseColor(prim->material, out_prim.base_color);

            int tex_index = TextureIndexFromMaterial(prim->material, data);
            if (tex_index >= 0 && tex_index < static_cast<int>(out_model.textures.size())) {
                out_prim.texture_id = out_model.textures[tex_index];
            }

            out_prim.vertices = std::move(vertices);
            out_prim.indices = std::move(indices);

            out_model.primitives.push_back(out_prim);
        }
    }

    cgltf_free(data);
    return !out_model.primitives.empty();
}

void DrawGltfModel(const GltfModel& model, Vec3 pos, float yaw_deg, float scale) {
    if (model.primitives.empty()) {
        return;
    }

    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);
    glRotatef(yaw_deg, 0.0f, 1.0f, 0.0f);
    glScalef(scale, scale, scale);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    const int stride = static_cast<int>(5 * sizeof(float));
    for (const auto& prim : model.primitives) {
        glPushMatrix();
        glMultMatrixf(prim.transform);

        glVertexPointer(3, GL_FLOAT, stride, prim.vertices.data());

        if (prim.has_uv) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, stride, prim.vertices.data() + 3);
        } else {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }

        if (prim.texture_id != 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, prim.texture_id);
        } else {
            glDisable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glColor4f(prim.base_color[0], prim.base_color[1], prim.base_color[2], prim.base_color[3]);
        glDrawElements(GL_TRIANGLES, prim.index_count, GL_UNSIGNED_INT, prim.indices.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        glPopMatrix();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void DestroyGltfModel(GltfModel& model) {
    for (auto& prim : model.primitives) {
        prim.vertices.clear();
        prim.indices.clear();
        prim.index_count = 0;
        prim.texture_id = 0;
    }
    for (auto& tex : model.textures) {
        if (tex != 0) {
            glDeleteTextures(1, &tex);
            tex = 0;
        }
    }
    model.primitives.clear();
    model.textures.clear();
}
