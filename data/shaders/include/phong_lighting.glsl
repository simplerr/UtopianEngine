
layout (std140, set = 0, binding = 1) uniform UBO_lights 
{
   // Constants
   float numLights;
   vec3 garbage;

   Light lights[10];
} light_ubo;

struct PhongMaterial
{
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
};

//! Computes the colors for directional light.
void ComputeDirectionalLight(PhongMaterial material, int lightIndex, vec3 normal, vec3 toEye, out vec4 ambient, out vec4 diffuse, out vec4 spec)
{
   Light light = light_ubo.lights[lightIndex];

   // Initialize outputs.
   ambient = vec4(0.0f, 0.0f, 0.0f, 1.0f);
   diffuse = vec4(0.0f, 0.0f, 0.0f, 1.0f);
   spec    = vec4(0.0f, 0.0f, 0.0f, 1.0f);

   // The light vector aims opposite the direction the light rays travel.
   vec3 lightVec = normalize(light.dir);

   // Add ambient term.
   ambient = material.ambient * light.color * light.intensity.x;

   float diffuseFactor = max(dot(lightVec, normal), 0.0f);

   vec3 v = reflect(lightVec, normal);
   float specFactor = pow(max(dot(v, toEye), 0.0f), material.specular.w);

   diffuse = diffuseFactor * material.diffuse * light.color * light.intensity.y;
   spec    = specFactor * material.specular * light.color * light.intensity.z;
}

//! Computes the colors for a point light.
void ComputePointLight(PhongMaterial material, int lightIndex, vec3 pos, vec3 normal, vec3 toEye, out vec4 ambient, out vec4 diffuse, out vec4 spec)
{
   Light light = light_ubo.lights[lightIndex];

   // Initialize outputs.
   ambient = vec4(0.0f, 0.0f, 0.0f, 0.0f);
   diffuse = vec4(0.0f, 0.0f, 0.0f, 0.0f);
   spec    = vec4(0.0f, 0.0f, 0.0f, 0.0f);

   // The vector from the surface to the light.
   // Todo: Note: Unclear
   pos.xyz *= -1.0f;
   normal.xz *= -1.0f;
   vec3 lightVec = light.pos - pos;
      
   // The distance from surface to light.
   float d = length(lightVec);

   // Range test.
   if( d > light.range )
      return;

   // Normalize the light vector.
   lightVec = normalize(lightVec);

   // Ambient term.
   ambient = material.ambient * light.color * light.intensity.x;

   // Add diffuse and specular term, provided the surface is in 
   // the line of site of the light.

   float diffuseFactor = max(dot(lightVec, normal), 0.0f);

   vec3 v = reflect(-lightVec, normal);
   float specFactor = pow(max(dot(v, toEye), 0.0f), material.specular.w);

   diffuse = diffuseFactor * material.diffuse * light.color * light.intensity.y;
   spec    = specFactor * material.specular * light.color * light.intensity.z;

   // Attenuate
   // See http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation for good constant values
   float att = 1.0f / dot(light.att, vec3(1.0f, d, d*d));

   ambient *= att;
   diffuse *= att;
   spec    *= att;
}

//! Computes the colors for a spot light.
void ComputeSpotLight(PhongMaterial material, int lightIndex, vec3 pos, vec3 normal, vec3 toEye, out vec4 ambient, out vec4 diffuse, out vec4 spec)
{
   Light light = light_ubo.lights[lightIndex];

   // Initialize outputs.
   ambient = vec4(0.0f, 0.0f, 0.0f, 0.0f);
   diffuse = vec4(0.0f, 0.0f, 0.0f, 0.0f);
   spec    = vec4(0.0f, 0.0f, 0.0f, 0.0f);

   // The vector from the surface to the light.
   // Todo: Note: Unclear
   pos.xyz *= -1.0f;
   normal.xz *= -1.0f;
   vec3 lightVec = light.pos - pos;
      
   // The distance from surface to light.
   float d = length(lightVec);
   
   // Range test.
   if(d > light.range)
      return;

   // Normalize the light vector.
   lightVec = normalize(lightVec);

   // Ambient term.
   ambient = material.ambient * light.color * light.intensity.x;   

   // Add diffuse and specular term, provided the surface is in 
   // the line of site of the light.

   float diffuseFactor = max(dot(lightVec, normal), 0.0f);

   vec3 v = reflect(-lightVec, normal);
   float specFactor = pow(max(dot(v, toEye), 0.0f), material.specular.w);
            
   diffuse = diffuseFactor * material.diffuse * light.color * light.intensity.y;
   spec    = specFactor * material.specular * light.color * light.intensity.z;
   
   // Scale by spotlight factor and attenuate.
   float spot = pow(max(dot(lightVec, normalize(light.dir)), 0.0f), light.spot);

   // Scale by spotlight factor and attenuate.
   float att = spot / dot(light.att, vec3(1.0f, d, d*d));

   ambient *= spot;
   diffuse *= att;
   spec    *= att;
}

//! Takes a list of lights and calculate the resulting color for the pixel after all light calculations.
void ApplyLighting(PhongMaterial material, vec3 posW, vec3 normalW, vec3 toEyeW, vec4 texColor,
                   float shadow, out vec4 litColor)
{
   // Start with a sum of zero.
   vec4 ambient = vec4(0.0f, 0.0f, 0.0f, 0.0f);
   vec4 diffuse = vec4(0.0f, 0.0f, 0.0f, 0.0f);
   vec4 spec    = vec4(0.0f, 0.0f, 0.0f, 0.0f);

   // Loop through all lights
   for(int i = 0; i < light_ubo.numLights; i++)
   {
      // Sum the light contribution from each light source.
      vec4 A, D, S;

      if(light_ubo.lights[i].type == 0.0f)         // Directional light
         ComputeDirectionalLight(material, i, normalW, toEyeW, A, D, S);
      else if(light_ubo.lights[i].type == 1.0f)    // Point light
         ComputePointLight(material, i, posW, normalW, toEyeW, A, D, S);
      else if(light_ubo.lights[i].type == 2.0f)    // Spot light
         ComputeSpotLight(material, i, posW, normalW, toEyeW, A, D, S);

      ambient += A;
      diffuse += shadow*D;
      spec    += shadow*S;
   }

   litColor = texColor*(ambient + diffuse) + spec;
}