#include <array>
#include <filesystem>
#include <fstream>
#include <print>
#include <utility>
#include <vector>

#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <toml++/toml.hpp>

#include "gl.h"

namespace fs = std::filesystem;

class GLFWContext {
public:
  GLFWContext() { glfwInit(); }
  ~GLFWContext() { glfwTerminate(); }
};

enum class AttribLocation : GLuint {
  position = 0,
};

enum class AttribType : GLenum {
  f32 = GL_FLOAT,
  f64 = GL_DOUBLE,
};

enum class IndexType : GLenum {
  u8 = GL_UNSIGNED_BYTE,
  u16 = GL_UNSIGNED_SHORT,
  u32 = GL_UNSIGNED_INT,
};

struct VertexAttribProps {
  AttribLocation location;
  AttribType type;
  size_t size;
};

struct VertexAttrib {
  VertexAttribProps props;
  size_t offset;
  bool normalized;
};

struct VertexFormat {
  std::vector<VertexAttrib> attribs;
  size_t stride;
};

// Abstract shader representation
// Contains input declarations for attributes and uniforms.
// Uniforms get populated by instantiating a Material referencing this Shader,
// and vertex attributes gets populated by a Mesh.
struct Shader {
  gl::Program program;
  std::vector<VertexAttribProps> attribs;
  // TODO: Uniforms props
};

struct Material {
  // should be a handle to shader instance but this works for now
  const Shader &shader;
  // TODO: Uniform bindings
};

struct VertexBuffer {
  gl::Buffer buffer;
  size_t offset;
  VertexFormat format;
};

struct IndexBuffer {
  gl::Buffer buffer;
  size_t offset{0};
  IndexType type{IndexType::u16};
};

enum class Primitive : GLenum { triangles = GL_TRIANGLES };

struct Mesh {
  // should be an identifier for the material instead of a reference
  const Material &material;
  gl::VAO vao;
  std::vector<VertexBuffer> vertex_buffers;
  size_t vector_count;
  Primitive primitive;
  std::optional<IndexBuffer> index_buffer{std::nullopt};
};

std::string read_file(const fs::path &path) {
  std::ifstream stream;
  std::string output;

  // find file size
  stream.open(path);
  stream.seekg(0, std::ios_base::end);
  auto len{stream.tellg()};
  stream.seekg(0, std::ios_base::beg);

  // prepare output storage
  output.resize(len);

  // read data to output from the beginning
  stream.read(output.data(), len);

  return output;
}

// Loads a shader from disk
// should be properly handled by an asset loader
Shader load_shader(std::string_view name) {
  // TODO: Read program shader names/types from metadata file
  // load fragment shader
  gl::Shader frag_shader{GL_FRAGMENT_SHADER};
  {
    fs::path frag_shader_path{std::format("{}.frag", name)};
    auto frag_source{read_file(frag_shader_path)};

    frag_shader.add_source(frag_source);
    frag_shader.compile();
  }

  // load vertex shader
  gl::Shader vert_shader{GL_VERTEX_SHADER};
  {
    fs::path vert_shader_path{std::format("{}.vert", name)};
    auto vert_source{read_file(vert_shader_path)};

    vert_shader.add_source(vert_source);
    vert_shader.compile();
  }

  // link shaders into program
  gl::Program program;
  program.attach_shader(frag_shader);
  program.attach_shader(vert_shader);
  program.link();
  // individual shaders will be deleted when they go out of scope

  return Shader(
      std::move(program),
      // TODO: Load attribs from metadata file
      {VertexAttribProps{
          .location = AttribLocation::position,
          .type = AttribType::f32,
          .size = 3 * sizeof(float),
      }}
  );
}

static std::array<float, 4 * 3> vertex_data = {
    0.5,
    -0.5,
    0.0,
    0.0,
    0.5,
    0.0,
    -0.5,
    -0.5,
    0.0,
};

static std::array<uint8_t, 3> index_data = {2, 1, 0};

Mesh load_mesh(std::string_view name, const Material &material) {
  // TODO: load data from disk
  gl::Buffer buffer;
  buffer.upload_data(vertex_data, GL_STATIC_DRAW);

  // TODO: load buffer formats from disk
  std::vector<VertexBuffer> vertex_buffers;
  vertex_buffers.emplace_back(
      std::move(buffer),
      0,
      VertexFormat{
          .attribs = {VertexAttrib{
              .props = VertexAttribProps(
                  AttribLocation::position,
                  AttribType::f32,
                  sizeof(float) * 3
              ),
              .offset = 0,
              .normalized = false,
          }},
          .stride = sizeof(float) * 3,
      }
  );

  // TODO: index data load from disk
  std::optional<IndexBuffer> index_buffer{std::in_place};
  index_buffer->type = IndexType::u8;
  index_buffer->buffer.upload_data(index_data, GL_STATIC_DRAW);

  // setup VAO (vertex array object)
  // VAO exposes binding indices for vertex buffers to supply data.
  gl::VAO vao;
  for (int buffer_idx{0}; buffer_idx < vertex_buffers.size(); ++buffer_idx) {
    const auto &vertex_buffer{vertex_buffers[buffer_idx]};

    // bind vertex buffer to its binding index
    glVertexArrayVertexBuffer(
        vao,
        buffer_idx,
        vertex_buffer.buffer,
        vertex_buffer.offset,
        vertex_buffer.format.stride
    );

    for (const auto &attrib : vertex_buffer.format.attribs) {
      auto attrib_index{static_cast<GLuint>(attrib.props.location)};
      auto attrib_type{static_cast<GLenum>(attrib.props.type)};
      auto attrib_size{static_cast<GLint>(attrib.props.size)};

      // configure position attrib, and assign its binding index
      glEnableVertexArrayAttrib(vao, attrib_index);
      glVertexArrayAttribBinding(vao, attrib_index, buffer_idx);
      glVertexArrayAttribFormat(
          vao,
          attrib_index,
          attrib_size,
          attrib_type,
          attrib.normalized,
          attrib.offset
      );
    }
  }

  if (index_buffer)
    glVertexArrayElementBuffer(vao, index_buffer->buffer);

  return Mesh{
      .material = material,
      .vao = std::move(vao),
      .vertex_buffers = std::move(vertex_buffers),
      .vector_count = 3,
      .primitive = Primitive::triangles,
      .index_buffer = std::move(index_buffer)
  };
}

void draw_mesh(const Mesh &mesh) {
  glUseProgram(mesh.material.shader.program);
  glBindVertexArray(mesh.vao);

  auto mode{static_cast<GLenum>(mesh.primitive)};
  if (mesh.index_buffer) {
    std::array commands = {gl::DrawElementsIndirectCommand{
        .count = static_cast<unsigned int>(mesh.vector_count),
        .instanceCount = 1,
        .firstIndex = 0,
        .baseVertex = 0,
        .baseInstance = 0,
    }};
    glMultiDrawElementsIndirect(
        mode,
        static_cast<GLenum>(mesh.index_buffer->type),
        commands.data(),
        commands.size(),
        0 // indicates structs are tightly packed
    );
  } else {
    glDrawArrays(mode, 0, static_cast<GLint>(mesh.vector_count));
  }
}

int main() {
  GLFWContext context{};

  auto window{glfwCreateWindow(800, 600, "Doodle", nullptr, nullptr)};

  glfwMakeContextCurrent(window);
  gladLoadGL(glfwGetProcAddress);

  float x_scale, y_scale;
  glfwGetWindowContentScale(window, &x_scale, &y_scale);
  glViewport(0, 0, 800 * x_scale, 600 * y_scale);

  auto shader{load_shader("main")};
  Material material{shader};
  auto mesh{load_mesh("triangle", material)};

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.21, 0.2, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    draw_mesh(mesh);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  glfwDestroyWindow(window);
}
