#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;

struct Material {
    sampler2D diffuse;
};

uniform Material material;
uniform float transparency;

void main()
{
    // 1. Obtenemos el color sólido de tu textura (rojo, verde, azul, etc.)
    vec4 texColor = texture(material.diffuse, TexCoords);
    
    // 2. Le damos un poco de iluminación ambiental para que no se vea plano
    // (Ajustamos basándonos en la luz de la Luna de tu escena)
    vec3 luzBase = vec3(0.6, 0.6, 0.7); 
    
    // 3. Pintamos el papel con su color multiplicado por la luz
    vec3 resultado = texColor.rgb * luzBase;
    
    FragColor = vec4(resultado, transparency);
}