#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;
uniform vec3 lightPos;


void main()
{
vec3 n = normalize(vertex_normal);
vec3 lp=lightPos;
vec3 ld = normalize(lp - vertex_pos);
float diffuse = dot(n,ld);

// map for neptune
color.rgb = vec3(0.65, 0.84, 1);

//color += texture(venusAtmos, vec2(vertex_tex.x, 1. - vertex_tex.y)).rgb;
color *= diffuse*0.8;

vec3 cd = normalize(vertex_pos - campos);
vec3 h = normalize(cd+ld);
float spec = dot(n,h);
spec = clamp(spec,0,1);
spec = pow(spec,40);

//color += vec4(1,1,1, 1)*spec;
if (length(vertex_pos) < 0.94)
{
    color.a = 0;
}
if (length(vertex_pos) >= 0.94)
{
    color.a = 0.4 + (1 - (length(vertex_pos)));
}
else
{
    color.a = 0.4;
}

}
