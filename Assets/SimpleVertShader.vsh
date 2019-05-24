#version 440 core

precision mediump float;

layout (location = 0) in vec3 inVertex;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBinormal;
layout (location = 4) in vec2 inUV0;
layout (location = 5) in vec2 inUV1;
layout (location = 6) in vec2 inUV2;

layout(std140) uniform Model
{
    uniform    mat4 ModelMatrix;
    uniform    mat4 ModelMatrix_IT;
    uniform    mat4 ModelMatrix_PreFrame;
    uniform    mat4 ModelMatrix_IT_PreFrame;
};

layout(std140) uniform View
{
    uniform    mat4 ViewMatrix;
    uniform    mat4 ProjectMatrix;
    
    uniform    mat4 ViewMatrix_PreFrame;
    uniform    mat4 ProjectMatrix_PreFrame;
    
    uniform    vec3 ViewPosition;
    uniform    vec2 ScreenSize;
};


out vec4 ClipSpacePos_Curr;
out vec4 ClipSpacePos_Pre;
out vec2 VelocityScreen;
out vec2 TexCoord0;
out vec2 TexCoord1;
out vec2 TexCoord2;
out vec2 SVPos;
out vec3 CameraPositionWorld;
out vec3 ViewDirWorld;
out vec3 PositionWorld;
out float Depth;
out mat3 TangentToWorldMatrix;
out mat4 FProjectMatrix;
out vec3 ClipPos;

out vec3 VDebugColor;

out vec3 NormalWorld;

vec3 LinearTosRGB(vec3 LinearColor)
{
    return (LinearColor.r < 0.0031308 && LinearColor.g < 0.0031308 && LinearColor.b < 0.0031308) ? 12.92 * LinearColor.rgb : 1.055 * pow(LinearColor.rgb, vec3(1.0 / 2.4)) - vec3(0.055, 0.055, 0.055);
}

void main()
{
    TexCoord0 = vec2(inUV0.x, inUV0.y);
    TexCoord1 = vec2(inUV1.x, inUV1.y);
    TexCoord2 = vec2(inUV2.x, inUV2.y);
    vec4 PosWorld = (ModelMatrix * vec4(inVertex, 1.0));
    ViewDirWorld = normalize(ViewPosition - PosWorld.xyz);
    CameraPositionWorld = ViewPosition;
    
    NormalWorld = normalize(mat3(ModelMatrix_IT)  * vec3(inNormal.x,inNormal.y,inNormal.z));
    vec3 TangentWorld = normalize(mat3(ModelMatrix_IT) * vec3(inTangent.x,inTangent.y,inTangent.z));
    vec3 BinormalWorld = normalize(mat3(ModelMatrix_IT) * vec3(inBinormal.x,inBinormal.y,inBinormal.z));
    //BinormalWorld = BinormalWorld * vec3(-1.0, 1.0, -1.0);
    
    TangentToWorldMatrix = mat3(TangentWorld.x, TangentWorld.y, TangentWorld.z,
                                BinormalWorld.x, BinormalWorld.y, BinormalWorld.z,
                                NormalWorld.x, NormalWorld.y, NormalWorld.z);
    
    FProjectMatrix = ProjectMatrix;
    
    PositionWorld = PosWorld.xyz;
    mat4 VP = ProjectMatrix * ViewMatrix;
    
    gl_Position = VP * PosWorld;
    Depth = gl_Position.w;
    //gl_Position = ProjectMatrix * ViewMatrix * ModelMatrix * vec4(inVertex, 1.0);
    
    SVPos = (gl_Position.xy / gl_Position.w * 0.5 + 0.5) * ScreenSize;
    
    ClipSpacePos_Curr = gl_Position;
    ClipSpacePos_Pre = ProjectMatrix * ViewMatrix_PreFrame * (ModelMatrix_PreFrame * vec4(inVertex, 1.0));
    VDebugColor = vec3(inNormal * 0.5 + 0.5);
    
    VelocityScreen = (gl_Position.xy / gl_Position.w * 0.5 + 0.5) - (ClipSpacePos_Pre.xy / ClipSpacePos_Pre.w * 0.5 + 0.5);
}
