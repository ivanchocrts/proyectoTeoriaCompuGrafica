#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

struct Material {
    sampler2D diffuse;
};

uniform Material material;

void main()
{
    vec4 texColor = texture(material.diffuse, TexCoords);
    
    // Solo descartamos si tiene transparencia real en PNG
    if(texColor.a < 0.1)
        discard;
        
    // Multiplicamos por 1.3 para que brille en la oscuridad sin quemar el color
    FragColor = vec4(texColor.rgb * 1.3, texColor.a);
}