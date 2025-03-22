#include "gl.h"

using std::convertible_to;
using std::string;
using std::string_view;

gl::Shader::Shader(GLenum type) { handle = glCreateShader(type); }

gl::Shader::Shader(Shader &&other) noexcept : handle(other.handle) {
  other.handle = 0;
}

gl::Shader::~Shader() { glDeleteShader(handle); }

gl::Shader &gl::Shader::operator=(Shader &&other) noexcept {
  // delete this object's handle
  glDeleteShader(handle);

  // move handle out of other into this
  handle = other.handle;
  other.handle = 0;

  return *this;
}

void gl::Shader::add_source(string_view source) const {
  auto source_data{source.data()};
  auto length{static_cast<GLint>(source.length())};
  glShaderSource(handle, 1, &source_data, &length);
}

void gl::Shader::compile() const {
  glCompileShader(handle);

  // retrieve compile status and check if it failed
  GLint compile_status;
  glGetShaderiv(handle, GL_COMPILE_STATUS, &compile_status);

  if (compile_status != GL_TRUE) {
    // get log length
    GLint log_length;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);

    // prepare log output storage
    std::string log;
    log.resize(log_length);

    // get log and actual log length
    GLint actual_length;
    glGetShaderInfoLog(handle, log_length, &actual_length, log.data());
    log.resize(actual_length);

    throw CompilationError(log);
  }
}

gl::Shader::operator GLuint() const { return handle; }

gl::Program::Program() : handle(glCreateProgram()) {}

gl::Program::Program(Program &&other) noexcept : handle(other.handle) {
  other.handle = 0;
}

gl::Program::~Program() { glDeleteProgram(handle); }

gl::Program &gl::Program::operator=(Program &&other) noexcept {
  // delete this object's handle
  glDeleteProgram(handle);

  // move handle out of other and into this
  handle = other.handle;
  other.handle = 0;

  return *this;
}

void gl::Program::attach_shader(GLuint shader) const {
  glAttachShader(handle, shader);
}

void gl::Program::link() const {
  glLinkProgram(handle);

  // retrieve compile status and check if it failed
  GLint link_status;
  glGetProgramiv(handle, GL_LINK_STATUS, &link_status);

  if (link_status != GL_TRUE) {
    // get log length
    GLint log_length;
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);

    // prepare log output storage
    string log;
    log.resize(log_length);

    // get log and actual log length
    GLint actual_length;
    glGetProgramInfoLog(handle, log_length, &actual_length, log.data());
    log.resize(actual_length);

    throw LinkError(log);
  }
}

gl::Program::operator unsigned int() const { return handle; }

gl::Buffer::Buffer() { glCreateBuffers(1, &handle); }

gl::Buffer::Buffer(Buffer &&other) noexcept : handle(other.handle) {
  other.handle = 0;
}

gl::Buffer::~Buffer() { glDeleteBuffers(1, &handle); }

gl::Buffer &gl::Buffer::operator=(Buffer &&other) noexcept {
  // delete this object's handle
  glDeleteBuffers(1, &handle);

  // move handle out of other and into this
  handle = other.handle;
  other.handle = 0;

  return *this;
}

void gl::Buffer::upload_data(const void *ptr, size_t size, GLenum usage) const {
  glNamedBufferData(handle, static_cast<GLsizeiptr>(size), ptr, usage);
}

gl::Buffer::operator unsigned int() const { return handle; }

gl::VAO::VAO() { glCreateVertexArrays(1, &handle); }

gl::VAO::VAO(VAO &&other) noexcept : handle(other.handle) { other.handle = 0; }

gl::VAO::~VAO() { glDeleteVertexArrays(1, &handle); }

gl::VAO &gl::VAO::operator=(VAO &&other) noexcept {
  // delete this object's handle
  glDeleteVertexArrays(1, &handle);

  // move handle out of other and into this
  handle = other.handle;
  other.handle = 0;

  return *this;
}

gl::VAO::operator unsigned int() const { return handle; }
