#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
uniform sampler2D texture_diffuse1;
uniform vec3 viewPos;
struct DirLight
{
    vec3 direction;
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

in vec3 fragPos;
uniform DirLight dirLight;
uniform SpotLight spotLight;

vec3 calcDirLight(DirLight dirLight, vec3 fragPos, vec3 viewPos, vec3 normals);
vec3 calculateSpotLight(SpotLight spotLight, vec3 fragPos, vec3 viewPos, vec3 normals);
void main()
{
    vec3 normals = normalize(Normal);

    vec3 result = vec3(0.0);

    result += calcDirLight(dirLight, fragPos, viewPos, normals);
    result += calculateSpotLight(spotLight, fragPos, viewPos, normals);

    result = pow(result, vec3(1.0/2.2));

    FragColor = vec4(result, 1.0) * texture(texture_diffuse1, TexCoords);
}

vec3 calcDirLight(DirLight dirLight, vec3 fragPos, vec3 viewPos, vec3 normals)
{
    //light beam direction for each fragment
    vec3 lightDir = normalize(dirLight.direction);

    //ambient
    float ambientStrength = 0.8;
    vec3 ambient = ambientStrength * dirLight.color;

    //diffuse
    float diff = max(dot(-lightDir, normals),0.0);
    vec3 diffuse = diff * dirLight.color;

    vec3 dir = ambient + diffuse;
    return dir;
}

vec3 calculateSpotLight(SpotLight spotLight, vec3 fragPos, vec3 viewPos, vec3 normals){

    //light beam direction for each fragment
    vec3 lightDir = normalize(fragPos - spotLight.position);

    //attenuation calcultation
    float distance = length(fragPos - spotLight.position); //not using lightDir because lightDir is normalized
    float attenuation = 1.0 / (spotLight.lightConst + spotLight.linearConst * distance + spotLight.quadraticConst * (distance*distance));

    //diffuse
    float diff = max(dot(-lightDir, normals),0.0);
    vec3 diffuse = diff * spotLight.color;
    diffuse *= attenuation;

    //specular
    float shiness = 32.0;
    float specularStrength = 0.1;

    vec3 reflectDir = reflect(-lightDir, normals);
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