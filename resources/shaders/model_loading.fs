#version 330 core

in vec2 texCords;
in vec3 aNormal;
in vec3 fragPos;

uniform sampler2D diffuse_texture1;
uniform sampler2D specular_texture1;

uniform vec3 viewPos;
uniform vec3 lightPosition;

out vec4 fragColor;

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
    vec3 norm = normalize(aNormal);

    vec3 result = vec3(0.0);
    result += calculateDirLight(dirLight, fragPos, viewPos, norm);
    result += calculatePointLight(pointLight, fragPos, viewPos, norm);
    result += calculateSpotLight(spotLight, fragPos, viewPos, norm);

    //Strange behaviour of gamma correction :|
    //result = pow(result,vec3(1.0/2.2));

    fragColor = vec4(result, 1.0f);
}

vec3 calculateSpotLight(SpotLight spotLight, vec3 fragPos, vec3 viewPos, vec3 norm){

    //light beam direction for each fragment
    vec3 lightDir = normalize(fragPos - spotLight.position);

    //attenuation calclutation
    float distance = length(fragPos - spotLight.position); //not using lightDir because lightDir is normalized
    float attenuation = 1.0 / (spotLight.lightConst + spotLight.linearConst * distance + spotLight.quadraticConst * (distance*distance));

    //diffuse
    float diff = max(dot(-lightDir, norm),0.0);
    vec3 diffuse = diff * spotLight.color * texture(diffuse_texture1, texCords).rgb;
    diffuse *= attenuation;

    //specular
    float shininess = 32.0;
    float specularStrength = 1.0;

    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 viewDir = normalize(fragPos - viewPos);

    float spec = pow(max(dot(-viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spotLight.color * spec * texture(specular_texture1, texCords).rgb;
    specular *= attenuation;

    //spot
    float cosTheta = dot(lightDir, normalize(spotLight.direction));
    float epsilon = spotLight.cutOff - spotLight.outerCutOff;
    float intensity = clamp((cosTheta - spotLight.outerCutOff)/epsilon, 0.0, 1.0);

    vec3 spot = vec3(0.0);

    if(spotLight.spotLightFlag == 1 && cosTheta > spotLight.outerCutOff){
        spot += (diffuse + specular) * intensity;
    }

    return spot;
}

vec3 calculatePointLight(PointLight pointLight, vec3 fragPos, vec3 viewPos, vec3 norm){

        vec3 lightDir = normalize(fragPos - pointLight.position);

        //attenuation calclutation
        float distance = length(fragPos - pointLight.position); //not using lightDir because lightDir is normalized
        float attenuation = 1.0 / (pointLight.lightConst + pointLight.linearConst * distance + pointLight.quadraticConst * (distance*distance));

        //ambient
        float ambientStrength = 0.2;
        vec3 ambient = ambientStrength * pointLight.color * texture(diffuse_texture1, texCords).rgb;
        ambient *= attenuation;

        //diffuse
        float diff = max(dot(-lightDir, norm), 0.0);
        vec3 diffuse = diff * pointLight.color * texture(diffuse_texture1, texCords).rgb;
        diffuse *= attenuation;

        //specular
        float shininess = 32.0;
        vec3 reflectDir = reflect(lightDir, norm);
        vec3 viewDir = normalize(fragPos - viewPos);
        float spec = pow(max(dot(-viewDir, reflectDir), 0.0), shininess);
        float specularStrength = 1.0;
        vec3 specular = specularStrength * dirLight.color * spec * texture(specular_texture1, texCords).rgb;
        specular *= attenuation;

        vec3 point = ambient + diffuse + specular;
        return point;
}

vec3 calculateDirLight(DirLight dirLight, vec3 fragPos, vec3 viewPos, vec3 norm){
    //light beam direction for each fragment
    vec3 lightDir = normalize(dirLight.direction);

    //ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * dirLight.color * texture(diffuse_texture1, texCords).rgb;

    //diffuse
    float diff = max(dot(-lightDir, norm),0.0);
    vec3 diffuse = diff * dirLight.color * texture(diffuse_texture1, texCords).rgb;

    //specular
    float shininess = 32.0;
    vec3 reflectDir = reflect(lightDir, norm);
    vec3 viewDir = normalize(fragPos - viewPos);
    float spec = pow(max(dot(-viewDir, reflectDir), 0.0), shininess);
    float specularStrength = 1.0;
    vec3 specular = specularStrength * dirLight.color * spec * texture(specular_texture1, texCords).rgb;

    vec3 dir = ambient + diffuse + specular;
    return dir;
}
