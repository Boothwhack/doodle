#pragma once

#include <format>
#include <stdexcept>

#include <glad/gl.h>

namespace gl {
class CompilationError : public std::runtime_error {
public:
  explicit CompilationError(const std::string &log)
      : std::runtime_error(std::format("Failed to compile shader: {}", log)) {}
};

class LinkError : public std::runtime_error {
public:
  explicit LinkError(const std::string &log)
      : std::runtime_error(std::format("Failed to link program: {}", log)) {}
};

class Shader {
  GLuint handle;

public:
  explicit Shader(GLenum type);
  Shader(const Shader &) = delete;
  Shader(Shader &&other) noexcept;
  ~Shader();

  Shader &operator=(const Shader &) = delete;
  Shader &operator=(Shader &&other) noexcept;

  void add_source(std::string_view source) const;

  void compile() const;

  // implicit conversion to GLuint OpenGL handle
  operator GLuint() const;
};

class Program {
  GLuint handle;

public:
  Program();
  Program(const Program &) = delete;
  Program(Program &&other) noexcept;
  ~Program();

  Program &operator=(const Program &) = delete;
  Program &operator=(Program &&other) noexcept;

  void attach_shader(GLuint shader) const;

  void link() const;

  // implicit conversion to GLuint OpenGL handle
  operator GLuint() const;
};

class Buffer {
  GLuint handle;

public:
  Buffer();
  Buffer(const Buffer &) = delete;
  Buffer(Buffer &&other) noexcept;
  ~Buffer();

  Buffer &operator=(const Buffer &) = delete;
  Buffer &operator=(Buffer &&other) noexcept;

  // accept any contiguous range of data
  void upload_data(std::ranges::contiguous_range auto data, GLenum usage) const {
    // calculate buffer size in bytes
    auto size{
        std::ranges::size(data) *
        sizeof(std::ranges::range_value_t<decltype(data)>)
    };
    // get constant pointer to beginning of data
    auto begin{std::ranges::cdata(data)};
    upload_data(begin, size, usage);
  }

  void upload_data(const void* ptr, size_t size, GLenum usage) const;

  // implicit conversion to GLuint OpenGL handle
  operator GLuint() const;
};

class VAO {
  GLuint handle{};

public:
  VAO();
  VAO(const VAO &) = delete;
  VAO(VAO &&other) noexcept;
  ~VAO();

  VAO &operator=(const VAO &) = delete;
  VAO &operator=(VAO &&other) noexcept;

  // implicit conversion to GLuint OpenGL handle
  operator GLuint() const;
};

typedef struct {
  uint count;
  uint instanceCount;
  uint firstIndex;
  int baseVertex;
  uint baseInstance;
} DrawElementsIndirectCommand;
} // namespace gl
