#version 450 core
out vec4 FragColor;

uniform vec3 camPos;
uniform mat4 inverseView;
uniform mat4 inverseProj;
uniform vec2 resolution;

layout (binding = 0) uniform usampler3D gridVolume;

#define GRID_SIZE 512

// calculate points at which the ray enters & exits the grid bounding box
vec2 intersectAABB(vec3 rayOrigin, vec3 rayDirection, vec3 boxMin, vec3 boxMax) {
   vec3 tMin = (boxMin - rayOrigin) / rayDirection;
   vec3 tMax = (boxMax - rayOrigin) / rayDirection;
   vec3 t1 = min(tMin, tMax);
   vec3 t2 = max(tMin, tMax);
   float tNear = max(max(t1.x, t1.y), t1.z);
   float tFar = min(min(t2.x, t2.y), t2.z);

   return vec2(tNear, tFar);
}

// on-the-fly ambient occlusion based on alive neighbors
float calculateAO(ivec3 hitPos, ivec3 normal) {
   int solidNeighbors = 0;

   // find tangent axes based on normal
   ivec3 t1 = normal.yzx;
   ivec3 t2 = normal.zxy;

   ivec3 surface = hitPos + normal;

   for(int i = -1; i <= 1; i++) {
      for(int j = -1; j <= 1; j++) {
         if(i == 0 && j == 0) continue;

         ivec3 samplePos = surface + (t1 * i) + (t2 * j);

         // wrap sample position to match the simulation
         samplePos = (samplePos + GRID_SIZE) % GRID_SIZE;

         uint cellData = texelFetch(gridVolume, samplePos, 0).r;
         solidNeighbors += int(cellData & 1u); // +1 if the cell is alive
      }
   }

   // # number of alive neighbors (0 - 8) -> brightness value (1.0 - 0.4)
   return 1.0 - (float(solidNeighbors) * 0.075);
}

void main()
{
   // pixel coordinate -> ray direction
   vec2 uv = (gl_FragCoord.xy / resolution.xy) * 2.0 - 1.0;
   vec4 target = inverseProj * vec4(uv.x, uv.y, 1.0, 1.0);
   vec3 rayDirection = normalize((inverseView * vec4(normalize(target.xyz / target.w), 0.0)).xyz);

   // assume grid is centered around (0, 0, 0) in the world
   vec3 rayOriginGrid = camPos + vec3(GRID_SIZE * 0.5);

   // bounding box intersection
   vec3 boxMin = vec3(0.0);
   vec3 boxMax = vec3(GRID_SIZE);
   vec2 hitData = intersectAABB(rayOriginGrid, rayDirection, boxMin, boxMax);

   float tNear = hitData.x; // step when we hit the near side
   float tFar = hitData.y; // step when we hit the far side

   // if ray misses the box we just draw the background
   if (tFar < 0.0 || tNear > tFar) {
      FragColor = vec4(0.05, 0.05, 0.08, 1.0);
      return;
   }

   // if the camera is outside the box, we advance the ray to the edge
   float t = max(0.0, tNear) + 0.001;
   vec3 currentPos = rayOriginGrid + rayDirection * t;

   // DDA setup
   vec3 safeRayDir = rayDirection + step(abs(rayDirection), vec3(0.000001)) * 0.000001;
   ivec3 mapPos = ivec3(floor(currentPos));
   vec3 deltaDist = abs(vec3(length(safeRayDir)) / safeRayDir);
   ivec3 rayStep = ivec3(sign(safeRayDir));
   vec3 sideDist = (sign(safeRayDir) * (vec3(mapPos) - currentPos) + (sign(safeRayDir) * 0.5) + 0.5) * deltaDist;

   ivec3 mask = ivec3(0);
   if (tNear > 0.0) {
      // Recalculate t1 from the AABB math
      vec3 t1 = min((boxMin - rayOriginGrid) / rayDirection, (boxMax - rayOriginGrid) / rayDirection);

      // Whichever axis has the largest t1 value is the face we hit first
      if (t1.x > t1.y && t1.x > t1.z) {
         mask = ivec3(1, 0, 0);
      } else if (t1.y > t1.z) {
         mask = ivec3(0, 1, 0);
      } else {
         mask = ivec3(0, 0, 1);
      }
   }
   bool hit = false;
   uint hitDataOut = 0;

   // DDA loop
   for (int i = 0; i < 1500; i++) {
      uint cellData = texelFetch(gridVolume, mapPos, 0).r;

      if ((cellData & 1u) == 1u) {
         hit = true;
         hitDataOut = cellData;
         break; // Stop marching!
      }

      // DDA step
      bvec3 b1 = lessThan(sideDist.xyz, sideDist.yzx);
      bvec3 b2 = lessThanEqual(sideDist.xyz, sideDist.zxy);
      mask = ivec3(b1) * ivec3(b2);
      sideDist += vec3(mask) * deltaDist;
      mapPos += mask * rayStep;

      // stop marching if we are outside the grid
      if (mapPos.x < 0 || mapPos.x >= GRID_SIZE ||
      mapPos.y < 0 || mapPos.y >= GRID_SIZE ||
      mapPos.z < 0 || mapPos.z >= GRID_SIZE) {
         break;
      }
   }

   // shading
   if (hit) {
      // normal is the inverse of te direction we stepped in when we hit the cell
      ivec3 normal = -mask * rayStep;

      // unpack neighbor count
      uint neighbors = (hitDataOut >> 1) & 31u;
      float heat = clamp(float(neighbors) / 26.0 * 1.5, 0.0, 1.0);

      // neighbor-driven color
      vec3 baseColor = mix(vec3(0.2, 0.8, 0.9), vec3(0.9, 0.2, 0.1), heat*0.8);

      // direcitonal light
      vec3 sunDirection = normalize(vec3(-0.8, -1.0, -0.3));
      float diffuse = max(dot(vec3(normal), -sunDirection), 0.1); // 0.1 is the ambient light

      // ambient occlusion
      float ao = calculateAO(mapPos, normal);

      // find distance for fog
      float dist = length((vec3(mapPos) + vec3(0.5)) - rayOriginGrid);

      vec3 finalColor = baseColor * diffuse * ao;

      // distance fog
       float fogFactor = exp(-dist * 0.0005);
      finalColor = mix(vec4(0.05, 0.05, 0.08, 1.0).rgb, finalColor, clamp(fogFactor, 0.0, 1.0));
      FragColor = vec4(finalColor, 1.0);
   } else {
      FragColor = vec4(0.05, 0.05, 0.08, 1.0); // backgroung color
   }
}