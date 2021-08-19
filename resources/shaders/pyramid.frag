#version 330 core

in vec2 texCords;
in vec3 aNormal;
in vec3 fragPos;

out vec4 fragColor;

uniform sampler2D texture_pyramid;
uniform vec3 viewPos;

struct DirLight{
    vec3 direction;
    vec3 color;
};

struct PointLight{
    float lightConst;
    float linearConst;
    float quadraticConst;

    vec3 position;
    vec3 color;
};

struct SpotLight{
    float lightConst;
    float linearConst;
    float quadraticConst;

    int spotLightFlag;
    vec3 position;
    vec3 direction;
    vec3 color;
    float cutOff;
    float outerCutOff;
};

uniform DirLight dirLight;
uniform PointLight pointLight;
uniform SpotLight spotLight;

vec3 calculateDirLight(DirLight dirLight, vec3 fragPos, vec3 viewPos, vec3 norm);
vec3 calculatePointLight(PointLight pointLight, vec3 fragPos, vec3 viewPos, vec3 norm);
vec3 calculateSpotLight(SpotLight spotLight, vec3 fragPos, vec3 viewPos, vec3 norm);

void main()
{

    //light normal
    vec3 norm = normalize(aNormal);

    vec3 result = vec3(0.0, 0.0, 0.0);

    result += calculateDirLight(dirLight, fragPos, viewPos, norm);
    result += calculatePointLight(pointLight, fragPos, viewPos, norm);
    result += calculateSpotLight(spotLight, fragPos, viewPos, norm);

    //gamma correction
    vec3 color = vec3(vec4(result, 1.0) * texture(texture_pyramid, texCords));
    color = pow(color,vec3(1.0/2.2));

    fragColor = vec4(color, 1.0);
}

vec3 calculateDirLight(DirLight dirLight, vec3 fragPos, vec3 viewPos, vec3 norm){
    //light beam direction for each fragment
    vec3 lightDir = normalize(dirLight.direction);

    //ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * dirLight.color;

    //diffuse
    float diff = max(dot(-lightDir, norm),0.0);
    vec3 diffuse = diff * dirLight.color;

    //specular
    float shinnes = 16;
    vec3 reflectDir = reflect(lightDir, norm);
    vec3 viewDir = normalize(fragPos - viewPos);
    float spec = pow(max(dot(-viewDir, reflectDir), 0.0), shinnes);
    float specularStrength = 0.2;
    vec3 specular = specularStrength * dirLight.color * spec;

    vec3 dir = ambient + diffuse + specular;
    return dir;
}

vec3 calculatePointLight(PointLight pointLight, vec3 fragPos, vec3 viewPos, vec3 norm){

    //light beam direction for each fragment
    vec3 lightDir = normalize(fragPos - pointLight.position);

    //attenuation calclutation
    float distance = length(fragPos - pointLight.position); //not using lightDir because lightDir is normalized
    float attenuation = 1.0 / (pointLight.lightConst + pointLight.linearConst * distance + pointLight.quadraticConst * (distance*distance));

    //diffuse
    float diff = max(dot(-lightDir, norm),0.0);
    vec3 diffuse = diff * pointLight.color;
    diffuse *= attenuation;

    //specular
    float shiness = 32;
    float specularStrength = 0.2;

    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 viewDir = normalize(fragPos - viewPos);

    float spec = pow(max(dot(-viewDir, reflectDir), 0.0), shiness);
    vec3 specular = specularStrength * pointLight.color * spec;
    specular *= attenuation;

    vec3 point = diffuse + specular;
    return point;
}

vec3 calculateSpotLight(SpotLight spotLight, vec3 fragPos, vec3 viewPos, vec3 norm){

    //light beam direction for each fragment
    vec3 lightDir = normalize(fragPos - spotLight.position);

    //attenuation calclutation
    float distance = length(fragPos - spotLight.position); //not using lightDir because lightDir is normalized
    float attenuation = 1.0 / (spotLight.lightConst + spotLight.linearConst * distance + spotLight.quadraticConst * (distance*distance));

    //diffuse
    float diff = max(dot(-lightDir, norm),0.0);
    vec3 diffuse = diff * spotLight.color;
    diffuse *= attenuation;

    //specular
    float shiness = 32;
    float specularStrength = 0.2;

    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 viewDir = normalize(fragPos - viewPos);

    float spec = pow(max(dot(-viewDir, reflectDir), 0.0), shiness);
    vec3 specular = specularStrength * spotLight.color * spec;
    specular *= attenuation;

    //spot
    float cosTheta = dot(lightDir, normalize(spotLight.direction));
    float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    float intensity = clamp((cosTheta - spotLight.outerCutOff)/epsilon, 0.0, 1.0);

    vec3 spot = vec3(0.0, 0.0, 0.0);

    if(spotLight.spotLightFlag == 1 && cosTheta > spotLight.outerCutOff){
        spot = (diffuse + specular) * intensity;
    }

    return spot;
}