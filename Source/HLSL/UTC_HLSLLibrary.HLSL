void MF_ComputeEdge(float3 CameraPos, float3 VertexNormal, out float3 Edges)
{
    float3 CamVertDir = normalize(CameraPos - VertexNormal);

    float3 DxNeg = clamp(cross(CamVertDir + ddx(CamVertDir), CamVertDir - ddx(CamVertDir)), 0, 1);
    float3 DxPos = clamp(cross(CamVertDir - ddx(CamVertDir), CamVertDir + ddx(CamVertDir)), 0, 1);
    float3 DyNeg = clamp(cross(CamVertDir + ddy(CamVertDir), CamVertDir - ddy(CamVertDir)), 0, 1);
    float3 DyPos = clamp(cross(CamVertDir - ddy(CamVertDir), CamVertDir + ddy(CamVertDir)), 0, 1);

    float3 Dx = DxNeg + DxPos;
    float3 Dy = DyNeg + DyPos;

    Edges = Dx + Dy;
}