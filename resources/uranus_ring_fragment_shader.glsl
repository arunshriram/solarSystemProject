#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;
uniform vec3 lightPos;

uniform sampler2D uraRingTex;

void main()
{

// map for saturn ring textures
vec3 n = normalize(vertex_normal);
vec3 ld = normalize(vertex_pos - lightPos);
float diffuse = dot(n,ld);

color = texture(uraRingTex, vertex_tex);
color.a = 0.3;


//color += texture(venusAtmos, vec2(vertex_tex.x, 1. - vertex_tex.y)).rgb;
//color *= diffuse*1.5;

}
