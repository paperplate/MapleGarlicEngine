module helpers;
// math helpers
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

// rendering helpers
public float4 drawShape(float4 bgColor, float2 uv, float2 center, float radius, float angle,
                        float4 fillColor, float outlineThickness, float2 resolution)
{
   float4 outlineColor = float4(0.1, 0.1, 0.1, 1.0);

   float2 p = uv - center;
   p = mul(p, rotate(-angle));
   float d = sdEllipse(p, radius);
   float aa = 2.0 / resolution.y;

   float fillAlpha = 1.0 - smoothstep(-aa, aa, d);
   float outlineAlpha = 1.0 - smoothstep(-aa, aa, d - outlineThickness);

   float4 result = lerp(bgColor, outlineColor, outlineAlpha);
   return lerp(result, fillColor, fillAlpha);
}
