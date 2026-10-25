#version 330 core

#define NUMBER_OF_POINT_LIGHTS 1

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NUMBER_OF_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;
uniform int transparency;

// Uniformes para animación de agua
uniform int isWater = 0;
uniform float uTime = 0.0;
uniform vec2 waterSpeed = vec2(0.02, 0.01); // Desplazamiento del mapa de textura

// Prototipos de funciones
vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir, vec2 uv );
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 uv );
vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 uv );

void main( )
{
    // Si el objeto es agua, desplaza progresivamente sus coordenadas UV
    vec2 finalTexCoords = TexCoords;
    if (isWater == 1)
    {
        finalTexCoords += uTime * waterSpeed;
    }

    vec3 norm = normalize( Normal );
    vec3 viewDir = normalize( viewPos - FragPos );
    
    // Iluminación Direccional (Sol)
    vec3 result = CalcDirLight( dirLight, norm, viewDir, finalTexCoords );
    
    // Luces Puntuales (Luna)
    for ( int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++ )
    {
        result += CalcPointLight( pointLights[i], norm, FragPos, viewDir, finalTexCoords );
    }
    
    // Spotlight
    result += CalcSpotLight( spotLight, norm, FragPos, viewDir, finalTexCoords );
    
    // Canal Alfa de la textura difusa
    float alphaChannel = texture(material.diffuse, finalTexCoords).a;
    color = vec4( result, alphaChannel );

    // Descarte de fragmentos transparentes (árboles PNG)
    if( color.a < 0.1 && transparency == 1 )
        discard;
}

vec3 CalcDirLight( DirLight light, vec3 normal, vec3 viewDir, vec2 uv )
{
    vec3 lightDir = normalize( -light.direction );
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    
    vec3 ambient = light.ambient * vec3( texture( material.diffuse, uv ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, uv ) );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, uv ) );
    
    return ( ambient + diffuse + specular );
}

vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 uv )
{
    vec3 lightDir = normalize( light.position - fragPos );
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    
    float distance = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant + light.linear * distance + light.quadratic * ( distance * distance ) );
    
    vec3 ambient = light.ambient * vec3( texture( material.diffuse, uv ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, uv ) );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, uv ) );
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return ( ambient + diffuse + specular );
}

vec3 CalcSpotLight( SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 uv )
{
    vec3 lightDir = normalize( light.position - fragPos );
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    vec3 reflectDir = reflect( -lightDir, normal );
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), material.shininess );
    
    float distance = length( light.position - fragPos );
    float attenuation = 1.0f / ( light.constant + light.linear * distance + light.quadratic * ( distance * distance ) );
    
    float theta = dot( lightDir, normalize( -light.direction ) );
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp( ( theta - light.outerCutOff ) / epsilon, 0.0, 1.0 );
    
    vec3 ambient = light.ambient * vec3( texture( material.diffuse, uv ) );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, uv ) );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, uv ) );
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    return ( ambient + diffuse + specular );
}