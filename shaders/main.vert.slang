struct VertexOutput
{
   float4 position : SV_Position;
   float2 uv : TEXCOORD;
};

[shader("vertex")]
VertexOutput main(uint vertexID: SV_VertexID)
{
   VertexOutput output;
   float x = -1.0 + float((vertexID & 1) << 2);
   float y = -1.0 + float((vertexID & 2) << 1);

   output.position = float4(x, y, 0.0, 1.0);
   output.uv = float2((x + 1.0) * 0.5, (y + 1.0) * 0.5);
   return output;
}
