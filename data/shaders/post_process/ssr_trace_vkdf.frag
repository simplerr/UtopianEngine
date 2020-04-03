#version 450

#extension GL_ARB_separate_shader_objects : enable

/*
   This SSR shader is inspired by https://github.com/itoral/vkdf,
   only minor changes have been done to integrate it in Utopian. 
*/

// layout(push_constant) uniform pcb
// {
//    mat4 Proj;
//    float aspect_ratio;
//    float tan_half_fov;
// } PCB;

// layout(set = 0, binding = 0) uniform sampler2D tex_color;
// layout(set = 0, binding = 1) uniform sampler2D tex_depth;
// layout(set = 0, binding = 2) uniform sampler2D tex_normal;

// layout(location = 0) in vec2 InTex;
// layout(location = 1) in vec2 in_view_ray;

// layout(location = 0) out vec4 OutFragColor;

layout (set = 0, binding = 0) uniform sampler2D lightSampler;
layout (set = 0, binding = 1) uniform sampler2D specularSampler;
layout (set = 0, binding = 2) uniform sampler2D normalViewSampler;
layout (set = 0, binding = 3) uniform sampler2D normalWorldSampler;
layout (set = 0, binding = 4) uniform sampler2D positionSampler;
layout (set = 0, binding = 5) uniform sampler2D depthSampler;

layout (location = 0) in vec2 InTex;

layout (location = 0) out vec4 OutFragColor;

layout (std140, set = 1, binding = 0) uniform UBO 
{
	mat4 view;
	mat4 projection;
} ubo;

/* While ray marching to find the reflection point, the minimum and
 * maximum step increments. Step sizes increment with each iteration starting
 * at the minimum step size up to the maximum. Making these smaller provides
 * better accuracy but requires more steps to get reflections coming from
 * pixels that are more distant from the fragment.
 *
 * Do notice that if we use variable step sizes then we can get artifacts
 * because the screen space distance between a fragment and its reflection
 * source changes when the camera moves into or away from the fragment, which
 * means that the step distances used for each way point on the reflection path
 * do not necessarily match and this can cause the maximum reflection
 * distance to change for the fragment between frames, leading to reflections
 * on the area close to the edge of the maximum reflection to pop in/out.
 */
layout (constant_id = 0)
const int   MAX_RAY_MARCH_MAX_SAMPLES            = 256;

layout (constant_id = 1)
const float RAY_MARCH_MIN_STEP_SIZE              = 0.001;

layout (constant_id = 2)
const float RAY_MARCH_MAX_STEP_SIZE              = 0.005;

/* While ray marching to find the reflection point, if we find a foreground
 * obstacle obstructing the path, the maximum number of iterations and
 * step size used to try and jump over it so we can continue ray marching
 * towards the reflection point.
 *
 * We consider an intersection a foreground obstacle if the depth of the
 * intersection is sufficiently smaller than our ray march point's depth minus
 * a bias.
 */
layout (constant_id = 3)
const float FOREGROUND_TEST_BIAS                 = 20.5;

layout (constant_id = 4)
const int   RAY_MARCH_OBSTACLE_MAX_SAMPLES       = 32;

layout (constant_id = 5)
const float RAY_MARCH_OBSTACLE_MIN_STEP_SIZE     = 0.01;

layout (constant_id = 6)
const float RAY_MARCH_OBSTACLE_MAX_STEP_SIZE     = 0.04;

/* While iterating to jump over a foreground obstacle, the minimum depth
 * increase we need to see on a jump sample to consider that we have
 * successfully moved past it.
 */
layout (constant_id = 7)
const float OBSTACLE_BREAK_DEPTH_DISTANCE        = 200.5;

/* When a foreground obstacle is very close to the camera it is probably
 * using up a big chunk of the screen space and trying to jump over it might
 * not make sense in that situation. If the distance from the intersection
 * point to the camera is smaller than this threshold, then we just fail
 * the reflection and don't attempt to jump over.
 */
layout (constant_id = 8)
const float OBSTACLE_JUMP_MIN_DISTANCE           = 100.0;

/* After finding a successul reflection candidate, the maximum number of
 * steps used to refine the actual reflection point. Increasing this improves
 * reflection quality at the expense of performance.
 */
layout (constant_id = 9)
const int   MAX_BINARY_SEARCH_SAMPLES            = 6;

/* Maximum distance (in screen space units) between the fragment and its
 * relfection source. If the distance exceed the limit, reflection is dropped.
 * (max == 2.0)
 */
layout (constant_id = 10)
const float MAX_REFLECTION_DISTANCE              = 2.0;

/* If the distance of the reflection is lower than the maximum threshold, then
 * its attenuated linearly between this distance and the maximum so that
 * reflections fade out smoothly with distance (max == MAX_REFLECTION_DISTANCE)
 */
layout (constant_id = 11)
const float ATT_REFLECTION_DISTANCE_START        = 0.25;

/* Distance from the screen edge (in screen space units) at which we start
 * attenuating reflections (max == 0.5)
 */
layout (constant_id = 12)
const float ATT_SCREEN_EDGE_DISTANCE_START       = 0.025;

/* If the direction of the reflection matches the normal of the surface
 * at the reflection point, then it means the relfection point is on the wrong
 * face of the polygon and the acual reflection point is not visible.
 * We drop these relections (max <= 1.0).
 */
layout (constant_id = 13)
const float MAX_DOT_REFLECTION_NORMAL            = 0.90;

/* Attenuate reflections as we get close to the maximum dot product between
 * the reflection direction and the normal direction at the reflection point.
 * We start attenuating when the dot product reaches this value.
 * (max == MAX_DOT_REFLECTION_NORMAL).
 */
layout (constant_id = 14)
const float ATT_DOT_REFLECTION_NORMAL_START      = 0.75;

/* Drop the reflection if it points back at the camera. This threshold
 * represents dot product of the reflection direction and the camera's
 * view direction at which we start dropping reflections.
 * (max == 1.0)
 */
layout (constant_id = 15)
const float MIN_DOT_REFLECTION_VIEW             = -0.75;

/* Dot product between the reflection direction and the camera's view diretion
 * at which we start attenuating reflections.
 * (min == MIN_DOT_REFLECTION_VIEW_START)
 */
layout (constant_id = 16)
const float ATT_DOT_REFLECTION_VIEW_START       = -0.25;

/* Output colors for various reasons we might fail to compute a valid
 * reflection point (these always have alpha 0 so they won't be applied
 * on to the final result). For debugging.
 */
#define ENABLE_DEBUG  0
#if ENABLE_DEBUG
const vec4  FAIL_COLOR_OBSTACLE                  = vec4(1.0, 0.0, 0.0, 1.0);
const vec4  FAIL_COLOR_DOUBLE_OBSTACLE           = vec4(1.0, 0.5, 0.0, 1.0);
const vec4  FAIL_COLOR_NOT_IN_SCREEN             = vec4(0.0, 1.0, 0.0, 1.0);
const vec4  FAIL_COLOR_REFLECTION_TOO_FAR        = vec4(0.0, 0.0, 1.0, 1.0);
const vec4  FAIL_COLOR_BACK_CAM                  = vec4(1.0, 1.0, 0.0, 1.0);
const vec4  FAIL_COLOR_REFLECTION_DIR            = vec4(0.0, 1.0, 1.0, 1.0);
const vec4  FAIL_COLOR_REFLECTION_DIST           = vec4(1.0, 1.0, 1.0, 1.0);
#else
const vec4  FAIL_COLOR_OBSTACLE                  = vec4(0.0);
const vec4  FAIL_COLOR_DOUBLE_OBSTACLE           = vec4(0.0);
const vec4  FAIL_COLOR_NOT_IN_SCREEN             = vec4(0.0);
const vec4  FAIL_COLOR_REFLECTION_TOO_FAR        = vec4(0.0);
const vec4  FAIL_COLOR_BACK_CAM                  = vec4(0.0);
const vec4  FAIL_COLOR_REFLECTION_DIR            = vec4(0.0);
const vec4  FAIL_COLOR_REFLECTION_DIST           = vec4(0.0);
#endif

float getDepth(vec2 texCoord)
{
   //return (ubo.view * vec4(texture(positionSampler, texCoord.xy).xyz, 1.0f)).z;
   return texture(depthSampler, texCoord).r;
}

// float
// eye_z_from_depth(sampler2D tex_depth, vec2 coord, mat4 Proj)
// {
//    float depth = texture(tex_depth, coord).x;
//    return -Proj[3][2] / (Proj[2][2] + depth);
// }

float
eye_z_from_depth(float depth, mat4 Proj)
{
   return -Proj[3][2] / (Proj[2][2] + depth);
}

vec3
eye_to_screen(vec3 pos)
{
   vec4 clip_pos   = ubo.projection * vec4(pos, 1.0);
   vec3 ndc_pos    = clip_pos.xyz / clip_pos.w;
   vec3 ss_pos     = vec3(ndc_pos.xy * vec2(0.5, 0.5) + vec2(0.5), ndc_pos.z);
   return ss_pos;
}

bool
is_outside_screen(vec2 p)
{
   return p.y < 0.0 || p.y > 1.0 || p.x < 0.0 || p.x > 1.0;
}

void
do_binary_search(vec3 ss_p_min, vec3 ss_p_max, out vec3 ss_p_out)
{
   float depth;
   for (int i = 0; i < MAX_BINARY_SEARCH_SAMPLES; i++) {
      vec3 ss_p = 0.5 * (ss_p_min + ss_p_max);
      //depth = texture(tex_depth, ss_p.xy).r;
      depth = getDepth(ss_p.xy);
      if (ss_p.z >= depth)
         ss_p_max = ss_p;
      else
         ss_p_min = ss_p;
   }

   ss_p_out = ss_p_max;
}

bool
is_foreground_obstacle(float z_buffer, float z, vec3 ss_max, vec3 ss_min)
{
   z_buffer = -eye_z_from_depth(z_buffer, ubo.projection);
   z = -eye_z_from_depth(z, ubo.projection);

   if (z_buffer < z - FOREGROUND_TEST_BIAS) {
      /* For reflection vectors where Z is the largest component, this can
       * happen easily, specially for relative large step sizes. In this cases
       * we want to refine the intersection point to make sure that it is a
       * real obstacle.
       */
      vec3 ss_out;
      do_binary_search(ss_min, ss_max, ss_out);
      z = -eye_z_from_depth(ss_out.z, ubo.projection);
      if (z_buffer < z - FOREGROUND_TEST_BIAS)
         return true;
   }

   return false;
}

/**
 * After we hit an obstacle in the reflection path, we take samples
 * further into the reflection path to test if we can jump over the obstacle.
 * This needs to account for how far away from the camera the obstacle is,
 * since the closer it is to the camera, the larger it is in screen space and
 * the larger the step size needs to be if we intend to go past it before
 * running out of jump samples.
 */
float
get_obstacle_jump_step_size(float z)
{
   z = -eye_z_from_depth(z, ubo.projection);
   return mix(RAY_MARCH_OBSTACLE_MIN_STEP_SIZE,
              RAY_MARCH_OBSTACLE_MAX_STEP_SIZE,
              clamp(0.0, 1.0, 1.0 / z));
}

/* If we have detected an intersection and we have identified it as a
 * foreground obstacle, we will see if it is a "small" obstacle and we
 * can just jump over it to continue on the reflection path.
 */
bool
handle_foreground_obstacle(vec3 ss_refl_dir,
                           inout vec3 ss_p,
                           float p_depth,
                           inout vec3 ss_prev_p,
                           inout bool keep_going)
{
   vec3 ss_orig_p = ss_p;
   vec3 ss_orig_prev_p = ss_prev_p;

   /* If the obstacle is too close to the camera it is probably using a big
    * chunk of the screen and we may fail to jump over, so let's just bail
    * early.
    */
   float p_dist = -eye_z_from_depth(p_depth, ubo.projection);
   if (p_dist < OBSTACLE_JUMP_MIN_DISTANCE) {
      OutFragColor = FAIL_COLOR_OBSTACLE;
      keep_going = false;
      return false;
   }

   float prev_jp_dist = p_dist;

   /* Compute the size of the jump step based on the depth of the obstacle */
   vec3 jump_step = ss_refl_dir * get_obstacle_jump_step_size(p_depth);

   for (int i = 0; i < RAY_MARCH_OBSTACLE_MAX_SAMPLES; i++) {
      /* Compute jump point along the ray march direction */
      vec3 ss_jp = ss_p + jump_step;

      /* If we stepped outside the screen it means one of:
       * 
       * a) The obstacle is at the screen edge and we cannot possibly find
       *    the reflection point past it.
       * b) The obstacle is very close to the camera and using up most of the
       *    screen space, occludding everything.
       *
       * Either way, we consider it a failed reflection. This is specially
       * important in case b), since otherwise when the obstacle is close
       * it pretty much casts its reflection everywhere and that is a very
       * visible artifact.
       *
       * FIXME: if it is case a) maybe we can return a reflection at ss_p.
       */
      if (is_outside_screen(ss_jp.xy)) {
         OutFragColor = FAIL_COLOR_NOT_IN_SCREEN;
         keep_going = false;
         return false;
      }

      /* Check for intersection at the jump point */
      //float jp_depth = texture(tex_depth, ss_jp.xy).r;
      float jp_depth = getDepth(ss_jp.xy);
      if (ss_jp.z >= jp_depth) {
         /* We found an intersection again. If the depth of the intersection
          * is larger enough than the original, then we assume that we
          * successfully jumped over the obstacle and we are now intersecting
          * another object behind it. We have a few options:
          *
          * a) We choose the jump point as reflection.
          * b) We choose the original point as reflection.
          * c) We fail the reflection.
          * d) We keep going.
          *
          * If we do a) we are always reflecting right past the edge of the
          * obstacle for every pixel that attempts to jump over. This can lead
          * to inconsistent reflections. It can also lead to incorrect
          * reflections if we managed to just go through some small geometry.
          *
          * If we do b) we are reflecting the original obstacle, so that
          * doesn't look good in general. This is specially true when the
          * obstacle is relatively close to the camera, since in that case it
          * casts its reflection over a large area making it very visible.
          *
          * c) can lead to very holes in the reflections that can be obvious.
          * 
          * d) usually leads to fail reflections in the end, but might depend
          * on the scene.
          *
          * For now, we choose c) since that seems to produce the best results,
          * but we might want to change this per scene.
          */
         float jp_dist = -eye_z_from_depth(jp_depth, ubo.projection);
         if (jp_dist - prev_jp_dist > OBSTACLE_BREAK_DEPTH_DISTANCE) {
#if 1 // case a)
            ss_p = ss_jp;
            ss_prev_p = ss_jp;
            return true;
#else // case c)
            OutFragColor = FAIL_COLOR_DOUBLE_OBSTACLE;
            keep_going = false;
            return false;
#endif
         }

         /* Otherwise, we consider that we are still hitting the same obstacle,
          * so we keep trying to move past it.
          */
         ss_p = ss_jp;
         ss_prev_p = ss_jp;
         prev_jp_dist = jp_dist;
         /* continue */
      } else {
         /* We don't have an intersection at the jump point. This means that we
          * jumped over the obstacle and we can continue searching for the
          * reflection point.
          */
         ss_p = ss_jp;
         ss_prev_p = ss_jp;
         return false;
      }
   }

   /* We did not manage to jump over the obstacle, so we consider it a
    * genuine reflection point.
    */
   ss_p = ss_orig_p;
   ss_prev_p = ss_orig_prev_p;
   return true;
}

/* Checks if we found an intersection at the current way point ss_p on the
 * reflection path. If keep_going is FALSE when the function also returns FALSE
 * it means that we should stop the process since we have assessed that we
 * can't find a reflection point.
 */
bool
find_intersection(inout vec3 ss_p,
                  inout vec3 ss_prev_p,
                  vec3 ss_refl_dir,
                  out float p_depth,
                  out bool keep_going)
{
   keep_going = true;
   //p_depth = texture(tex_depth, ss_p.xy).r;
   p_depth = getDepth(ss_p.xy);
   if (ss_p.z >= p_depth) {
      /* Found intersection. Now we need to check if it is a foreground
       * obstacle that is obstructing the reflection path.
       */
      if (is_foreground_obstacle(p_depth, ss_p.z, ss_p, ss_prev_p)) {
         /* It is a foregound obstacle, check if it is small enough that we can
          * jump over it, in which case we can continue looking for our
          * reflection point.
          */
         return handle_foreground_obstacle(ss_refl_dir,
                                           ss_p,
                                           p_depth,
                                           ss_prev_p,
                                           keep_going);
      }

      /* Our intersection point is not a foreground obstacle, so we consider
       * it a genuine reflection point
       */
      return true;
   }

   /* We did not find any intersection at this point, so keep looking for it */
   return false;
}

/* Figures out the best step size to use. This is rather tricky, since too small
 * steps require long paths and have a large perf impact, on the other side,
 * large steps significantly reduce quality and lead to more artifacts, so we
 * take into account the depth distance between our current way point and the
 * depth buffer to modulate the step size, using larger step sizes when we
 * seem to be at larger distances from obstacles.
 */
float
get_step_size(int step_idx, vec3 p, float p_depth)
{
   if (step_idx == 0)
      return RAY_MARCH_MAX_STEP_SIZE;

   float z_buffer = -eye_z_from_depth(p_depth, ubo.projection);
   float z = -eye_z_from_depth(p.z, ubo.projection);
   float dist = z_buffer - z;

   if (dist < 3.0)
      return RAY_MARCH_MIN_STEP_SIZE;

   return RAY_MARCH_MAX_STEP_SIZE;
}

/* Gets the reflection position for this fragment. If it fails to compute
 * a valid reflection position and debug mode is enabled, then a debug color
 * is written to identify the cause of the reflection failure.
 */
bool
get_reflection_position(vec3 ss_refl_dir, vec3 ss_p_min, out vec3 ss_p_out)
{
   vec3 ss_p_max;
   bool keep_going = true;

   float p_depth = 1.0;
   vec3 ss_p = ss_p_min;

   for (int i = 0; i < MAX_RAY_MARCH_MAX_SAMPLES; i++) {
      /* Compute new sample along the reflection path. If the sample is outside
       * the screen, then we failed to find the reflection point.
       */
      ss_p = ss_p_min + get_step_size(i, ss_p, p_depth) * ss_refl_dir;
      if (is_outside_screen(ss_p.xy)) {
         OutFragColor = FAIL_COLOR_NOT_IN_SCREEN;
         return false;
      }

      /* Check if we found an intersection at this sample
       */
      if (find_intersection(ss_p, ss_p_min, ss_refl_dir, p_depth, keep_going)) {
         /* We found a reflection candidate, let's refine its position
          * with a binary search.
          */
         ss_p_max = ss_p;
         do_binary_search(ss_p_min, ss_p_max, ss_p_out);
         return true;
      } else {
         /* We did not hit an intersection.
          *
          * If keep_going is false it means that we have concluded that no
          * reflection is available, so we should stop the process now.
          */
         if (!keep_going)
            return false;

         ss_p_min = ss_p;
      }
   }

   /* We reached the maximum number of steps without hitting a valid reflection
    * point.
    */
   OutFragColor = FAIL_COLOR_REFLECTION_TOO_FAR;
   return false;
}

void main()
{
   /* Use all inputs so they don't get optimized away */
   vec4 tmp = texture(lightSampler, InTex);

   /* Bail out if this fragment is not reflective */
   vec3 normal = texture(normalWorldSampler, InTex).rgb;
   vec4 specular = texture(specularSampler, InTex);
   float reflectiveness = specular.a;

   if (reflectiveness < 0.4f || normal.y < 0.7f)
   {
      OutFragColor = vec4(0.0);
      return;
   }

   /* Compute camera to fragment vector */
   // float eye_position_z = eye_z_from_depth(tex_depth, InTex, ubo.projection);
   // float eye_position_x = in_view_ray.x * eye_position_z;
   // float eye_position_y = in_view_ray.y * eye_position_z;
   // vec3 eye_position = vec3(eye_position_x, eye_position_y, eye_position_z);
   // vec3 eye_viewdir = normalize(eye_position);

   // /* Compute reflection vector */
   // vec3 eye_normal = texture(tex_normal, InTex).rgb;
   // vec3 eye_refl_dir = reflect(eye_viewdir, eye_normal);

   vec3 world_position = texture(positionSampler, InTex).xyz;
   vec3 eye_position = (ubo.view * vec4(world_position, 1.0f)).xyz;
   vec3 eye_viewdir = normalize(eye_position);
   vec3 eye_normal = normalize(texture(normalViewSampler, InTex).xyz * 2.0f - 1.0f);
   vec3 eye_refl_dir = reflect(eye_viewdir, eye_normal);

   /* Compute reflection vector */
   // vec3 normalView = normalize(texture(normalViewSampler, InTex).xyz * 2.0f - 1.0f);
   // vec3 positionView = (ubo.view * vec4(positionWorld, 1.0f)).xyz;
   // vec3 reflectionDir = normalize(reflect(normalize(positionView), normalize(normalView)));

   /* If the reflection angle is coming back at the camera, progressively
    * fade it out.
    */
   float att_cam = 1.0;
#if 0 // FIXME: this doesn't quite work as intended, needs to be reviewed
   float dp_view_refl = dot(eye_viewdir, eye_refl_dir);
   if (dp_view_refl < MIN_DOT_REFLECTION_VIEW) {
      OutFragColor = FAIL_COLOR_BACK_CAM;
      return;
   } else if (dp_view_refl < ATT_DOT_REFLECTION_VIEW_START) {
      const float att_range =
         ATT_DOT_REFLECTION_VIEW_START - MIN_DOT_REFLECTION_VIEW;
      const float att_step = ATT_REFLECTION_DISTANCE_START - dp_view_refl;
      att_cam =  1.0 - att_step / att_range;
   }
#endif

   /* Compute screen space position of this fragment */
   //vec3 ss_frag_pos = vec3(InTex, texture(tex_depth, InTex));
   vec3 ss_frag_pos = vec3(InTex, getDepth(InTex));

   /* Compute a point along reflection vector and use it to extract
    * screen-space reflection vector
    */
   vec3 eye_p = eye_position + 5.0 * eye_refl_dir;
   vec3 ss_p = eye_to_screen(eye_p);
   vec3 ss_refl_dir = normalize(ss_p - ss_frag_pos);

   /* Get initial reflection point candidate */
   vec3 ss_p_start = ss_frag_pos;
   if (!get_reflection_position(ss_refl_dir, ss_p_start, ss_p)) {
      /* Debug output color should have already been set */
      return;
   }

   /* Attenuate with distance from the fragment */
   float att_refl_dist = 1.0;
   float refl_dist = length(ss_p - ss_p_start);
   if (refl_dist > MAX_REFLECTION_DISTANCE) {
      OutFragColor = FAIL_COLOR_REFLECTION_DIST;
      return;
   } else if (refl_dist > ATT_REFLECTION_DISTANCE_START) {
      const float att_range =
         MAX_REFLECTION_DISTANCE - ATT_REFLECTION_DISTANCE_START;
      const float att_step = refl_dist - ATT_REFLECTION_DISTANCE_START;
      att_refl_dist = 1.0 - att_step / att_range;
   }

   /* Attenuate as we get closer to the screen edge */
   float att_uv = 1.0;
   const float screen_edge_min = ATT_SCREEN_EDGE_DISTANCE_START;
   const float screen_edge_max = 1.0 - ATT_SCREEN_EDGE_DISTANCE_START;
   if (ss_p.x < screen_edge_min || ss_p.x > screen_edge_max ||
       ss_p.y < screen_edge_min || ss_p.y > screen_edge_max) {
      const float att_range = ATT_SCREEN_EDGE_DISTANCE_START;
      const vec2 att_step = (0.5 - abs(ss_p.xy - 0.5)) / att_range;
      att_uv = min(att_step.x, att_step.y);
   }

   /* Only accept reflections if the reflection direction is not too similar
    * to the normal at the reflection point (since that means the reflection
    * point is on the wrong face of the polygon.
    */
   float att_refl_angle = 1.0;
   //vec3 eye_p_normal = texture(tex_normal, ss_p.xy).rgb;
   vec3 eye_p_normal = texture(normalViewSampler, ss_p.xy).rgb;
   float dp_normal_refl_dir = dot(eye_refl_dir, eye_p_normal);
   if (dp_normal_refl_dir > MAX_DOT_REFLECTION_NORMAL) {
      OutFragColor = FAIL_COLOR_REFLECTION_DIR;
      return;
   } else if (dp_normal_refl_dir > ATT_DOT_REFLECTION_NORMAL_START) {
      const float att_range =
         MAX_DOT_REFLECTION_NORMAL - ATT_DOT_REFLECTION_NORMAL_START;
      const float att_step =
         dp_normal_refl_dir - ATT_DOT_REFLECTION_NORMAL_START;
      att_refl_angle = att_step / att_range;
   }

   OutFragColor.rgb = texture(lightSampler, ss_p.xy).rgb;
   OutFragColor.a = reflectiveness * att_cam * att_uv * att_refl_angle * att_refl_dist;
}