module helpers;

//// Math and SDF Helpers ////
public float2x2 rotate(float angle)
{
   float s = sin(angle);
   float c = cos(angle);
   return float2x2(c, -s, s, c);
}

public float sdEllipse(float2 p, float2 r)
{
   float k = length(p / r);
   return length(p) * (1.0 - 1.0 / k);
}

public float sdBox(float2 p, float2 b)
{
   float2 d = abs(p) - b;
   return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

// where r is the radius of the bounding circle
public float sdEquilateralTriangle(float2 p, float r)
{
   const float k = 1.7320508; // sqrt(3)
   p.x = abs(p.x) - r;
   p.y = p.y + r / k;
   if (p.x + k * p.y > 0.0)
   {
      p = float2(p.x - k * p.y, -k * p.x - p.y) / 2.0;
   }
   p.x -= clamp(p.x, -2.0 * r, 0.0);
   return -length(p) * sign(p.y);
}

// Line segment between points a and b
public float sdLine(float2 p, float2 a, float2 b)
{
   float2 pa = p - a;
   float2 ba = b - a;
   float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
   return length(pa - ba * h);
}

// Quadratic Bezier Curve
public float sdBezier(float2 pos, float2 start, float2 control, float2 end)
{
   const float sqrt3 = 1.7320508; // sqrt(3)
   float2 a = control - start;
   float2 b = start - 2.0 * control + end;
   float2 c = a * 2.0;
   float2 d = start - pos;

   float kk = 1.0 / dot(b, b);
   float kx = kk * dot(a, b);
   float ky = kk * (2.0 * dot(a, a) + dot(d, b)) / 3.0;
   float kz = kk * dot(d, a);

   float res = 0.0;
   float p = ky - kx * kx;
   float p3 = p * p * p;
   float q = kx * (2.0 * kx * kx - 3.0 * ky) + kz;
   float h = q * q + 4.0 * p3;

   if (h >= 0.0)
   {
      h = sqrt(h);
      float2 x = (float2(h, -h) - q) / 2.0;
      float2 uv = sign(x) * pow(abs(x), float2(1.0 / 3.0));
      float t = clamp(uv.x + uv.y - kx, 0.0, 1.0);
      res = length(d + (c + b * t) * t);
   }
   else
   {
      float z = sqrt(-p);
      float v = acos(q / (p * z * 2.0)) / 3.0;
      float m = cos(v);
      float n = sin(v) * sqrt3;
      float3 t = clamp(float3(m + m, -n - m, n - m) * z - kx, 0.0, 1.0);
      res = min(length(d + (c + b * t.x) * t.x), length(d + (c + b * t.y) * t.y));
   }
   return res;
}

//// Rendering Helpers ////

// Reusable anti-aliasing and coloring logic for any shape distance
public float4 resolveShapeColor(float4 bgColor, float d, float4 fillColor, float outlineThickness,
                                float2 resolution)
{
   float4 outlineColor = float4(0.1, 0.1, 0.1, 1.0);
   float aa = 2.0 / resolution.y;

   float fillAlpha = 1.0 - smoothstep(-aa, aa, d);
   float outlineAlpha = 1.0 - smoothstep(-aa, aa, d - outlineThickness);

   float4 result = lerp(bgColor, outlineColor, outlineAlpha);
   return lerp(result, fillColor, fillAlpha);
}

public float4 drawShape(float4 bgColor, float2 uv, float2 center, float2 radius, float angle,
                        float4 fillColor, float outlineThickness, float2 resolution)
{
   float2 p = mul(uv - center, rotate(-angle));
   float d = sdEllipse(p, radius);
   return resolveShapeColor(bgColor, d, fillColor, outlineThickness, resolution);
}

public float4 drawBox(float4 bgColor, float2 uv, float2 center, float2 size, float angle,
                      float4 fillColor, float outlineThickness, float2 resolution)
{
   float2 p = mul(uv - center, rotate(-angle));
   float d = sdBox(p, size);
   return resolveShapeColor(bgColor, d, fillColor, outlineThickness, resolution);
}

public float4 drawTriangle(float4 bgColor, float2 uv, float2 center, float radius, float angle,
                           float4 fillColor, float outlineThickness, float2 resolution)
{
   float2 p = mul(uv - center, rotate(-angle));
   float d = sdEquilateralTriangle(p, radius);
   return resolveShapeColor(bgColor, d, fillColor, outlineThickness, resolution);
}

// Line thickness is subtracted from length, no extra outline drawn
public float4 drawLine(float4 bgColor, float2 uv, float2 start, float2 end, float4 lineColor,
                       float lineThickness, float2 resolution)
{
   float d = sdLine(uv, start, end) - (lineThickness / 2.0);
   return resolveShapeColor(bgColor, d, lineColor, 0.0, resolution);
}

public float4 drawCurve(float4 bgColor, float2 uv, float2 start, float2 control, float2 end,
                        float4 lineColor, float lineThickness, float2 resolution)
{
   float d = sdBezier(uv, start, control, end) - (lineThickness / 2.0);
   return resolveShapeColor(bgColor, d, lineColor, 0.0, resolution);
}
