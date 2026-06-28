#version 330 compatibility

in vec3 fragPos;
in vec3 fragNormal;
in vec2 texCoord;

uniform sampler2D diffuseMap;
uniform int hasTexture;

out vec4 FragColor;

void main()
{
    vec3 baseColor = vec3(1.0);

    if (hasTexture == 1)
        baseColor = texture(diffuseMap, texCoord).rgb;

    vec3 lightDir = normalize(vec3(0.0, 10.0, 10.0) - fragPos);
    float diff = max(dot(normalize(fragNormal), lightDir), 0.0);

    vec3 ambient = baseColor * 0.55;
    vec3 diffuse = baseColor * diff * 0.65;

    FragColor = vec4(ambient + diffuse, 1.0);
}