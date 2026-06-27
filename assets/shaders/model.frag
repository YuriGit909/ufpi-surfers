#version 120

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform sampler2D roughnessMap;

uniform int hasDiffuse;
uniform int hasNormal;
uniform int hasSpecular;
uniform int hasRoughness;

uniform vec4 materialColor;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 ambientColor;

varying vec3 fragPos;
varying vec3 fragNormal;
varying vec2 texCoord;
varying mat3 TBN;

void main()
{
    vec4 baseColor = materialColor;

    if (hasDiffuse == 1)
        baseColor = texture2D(diffuseMap, texCoord);

    if (baseColor.a < 0.05)
        discard;

    vec3 normal = normalize(fragNormal);

    if (hasNormal == 1)
    {
        vec3 sampledNormal = texture2D(normalMap, texCoord).rgb;
        sampledNormal = sampledNormal * 2.0 - 1.0;
        normal = normalize(TBN * sampledNormal);
    }

    vec3 lightDir = normalize(lightPosition - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 viewDir = normalize(-fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);

    float shininess = 32.0;
    if (hasRoughness == 1)
    {
        float roughness = texture2D(roughnessMap, texCoord).g;
        shininess = mix(96.0, 8.0, roughness);
    }

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specularColor = vec3(0.35);

    if (hasSpecular == 1)
        specularColor *= texture2D(specularMap, texCoord).rgb;

    vec3 ambient = ambientColor * baseColor.rgb;
    vec3 diffuse = diff * lightColor * baseColor.rgb;
    vec3 specular = spec * lightColor * specularColor;

    gl_FragColor = vec4(ambient + diffuse + specular, baseColor.a);
}
