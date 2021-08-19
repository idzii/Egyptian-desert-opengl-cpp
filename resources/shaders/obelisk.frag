#version 330 core

in vec2 texCords;
in vec3 aNormal;
in vec3 fragPos;

out vec4 fragColor;

uniform vec3 viewPos;

struct Material{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

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

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLight;
uniform SpotLight spotLight;

vec3 calculateDirLight(DirLight dirLight, Material materijal, vec3 fragPos, vec3 viewPos, vec3 norm);
vec3 calculatePointLight(PointLight pointLight, Material material, vec3 fragPos, vec3 viewPos, vec3 norm);
vec3 calculateSpotLight(SpotLight spotLight, Material material, vec3 fragPos, vec3 viewPos, vec3 norm);

void main()
{

    //light normal
    vec3 norm = normalize(aNormal);

    vec3 result = vec3(0.0, 0.0, 0.0);

    //beams are much stronger light than spotlight and pointlight so they are not affected by these lights
    result += calculateDirLight(dirLight, material, fragPos, viewPos, norm);
    //result += calculatePointLight(pointLight, material, fragPos, viewPos, norm);
    //result += calculateSpotLight(spotLight, material, fragPos, viewPos, norm);

    fragColor = vec4(result, 1.0);
}

vec3 calculateDirLight(DirLight dirLight, Material material, vec3 fragPos, vec3 viewPos, vec3 norm){
    //light beam direction for each fragment
    vec3 lightDir = normalize(dirLight.direction);

    //ambient
    float ambientStrength = 0.4;
    //vec3 ambient = ambientStrength * dirLight.color * material.ambient;
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0) * material.ambient;

    //diffuse
    //float diff = max(dot(-lightDir, norm),0.0);
    float diff = 1.0;
    //vec3 diffuse = diff * dirLight.color * material.diffuse;
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0) * material.diffuse; //so beams are not dependent on sun light color

    //specular
    vec3 viewDir = normalize(fragPos - viewPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(-viewDir, reflectDir), 0.0), material.shininess);

    float specularStrength = 1.0;
    //vec3 specular = specularStrength * dirLight.color * material.specular * spec;
    vec3 specular = specularStrength * vec3(1.0, 1.0, 1.0) * material.specular * spec;

    vec3 dir = ambient + diffuse + specular;
    return dir;
}

vec3 calculatePointLight(PointLight pointLight, Material material, vec3 fragPos, vec3 viewPos, vec3 norm){

    //light beam direction for each fragment
    vec3 lightDir = normalize(fragPos - pointLight.position);

    //attenuation calclutation
    float distance = length(fragPos - pointLight.position); //not using lightDir because lightDir is normalized
    float attenuation = 1.0 / (pointLight.lightConst + pointLight.linearConst * distance + pointLight.quadraticConst * (distance*distance));

    //diffuse
    float diff = max(dot(-lightDir, norm),0.0);
    vec3 diffuse = diff * pointLight.color * material.diffuse;
    diffuse *= attenuation;

    //specular
    float specularStrength = 0.5;

    vec3 viewDir = normalize(fragPos - viewPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(1.0 - max(dot(-viewDir, reflectDir), 0.0), material.shininess);

    vec3 specular = specularStrength * pointLight.color * spec * material.specular;
    specular *= attenuation;

    vec3 point = diffuse + specular;
    return point;
}

vec3 calculateSpotLight(SpotLight spotLight, Material material, vec3 fragPos, vec3 viewPos, vec3 norm){

    //light beam direction for each fragment
    vec3 lightDir = normalize(fragPos - spotLight.position);

    //attenuation calclutation
    float distance = length(fragPos - spotLight.position); //not using lightDir because lightDir is normalized
    float attenuation = 1.0 / (spotLight.lightConst + spotLight.linearConst * distance + spotLight.quadraticConst * (distance*distance));

    //diffuse
    float diff = max(dot(-lightDir, norm),0.0);
    vec3 diffuse = diff * spotLight.color * material.diffuse;
    diffuse *= attenuation;

    //specular
    float specularStrength = 0.5;

    vec3 viewDir = normalize(fragPos - viewPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(1.0 - max(dot(-viewDir, reflectDir), 0.0), material.shininess);

    vec3 specular = specularStrength * spotLight.color * spec * material.specular;
    specular *= attenuation;

    //spot
    float cosTheta = dot(lightDir, normalize(spotLight.direction));
    float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    float intensity = clamp((cosTheta - spotLight.outerCutOff)/epsilon, 0.0, 1.0);
    float spotDistance = length(spotLight.position - fragPos);
    float spotAttenuation = 1.0 / (spotLight.lightConst + spotLight.linearConst * spotDistance + spotLight.quadraticConst * (spotDistance*spotDistance));

    vec3 spot = vec3(0.0, 0.0, 0.0);

    if(spotLight.spotLightFlag == 1 && cosTheta > spotLight.outerCutOff){
        spot = (diffuse + specular) * intensity;
    }

    return spot;
}