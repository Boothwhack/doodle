#version 440 core
layout (location = 0) in vec3 a_Pos; // the position variable has attribute position 0

layout (std140, binding = 0) uniform Matrices {
    mat4 u_ProjView;
};

out vec4 vertexColor; // specify a color output to the fragment shader

void main()
{
    gl_Position = u_ProjView * vec4(a_Pos, 1.0);
    vertexColor = vec4(0.5, 0.0, 0.0, 1.0); // set the output variable to a dark-red color
}
